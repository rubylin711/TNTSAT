/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
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

//#include "mt_type.h" //flax

//#include <drv_frontend.h>
#include "mt_unf_frontend.h"
//#include "drv_frontend.h"
//#include "../drv_frontend_priv.h"
//#include "mt_fe_def_cs8000_sat.h"
#include "mt_fe_sat_tn_montage_ts6011.h"
//#include "mt_fe_sat_tn_montage_ts2022.h"
#include "mt_fe_common_cs8000_sat.h"
#include "port_m88cs8k_sat.h"
#include "drv_frontend_ioctl.h"
#include "mt_module_debug.h"
#include "mt_drv_sys.h"
//#define FOR_PORT_CS8K_SAT_CONNECT_SYCHRONOUS

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

#define MT_FE_CS8K_SAT_PIN_LEVEL_LOW 0
#define MT_FE_CS8K_SAT_PIN_LEVEL_HIGH 1
//#define MT_FE_CS8K_SAT_DISEQC_COMMAND_START_DELAY 30
#define MT_FE_CS8K_SAT_DISEQC_COMMAND_START_DELAY 50
#define MT_FE_CS8K_SAT_DISEQC_COMMAND_END_DELAY 50

#define MAX_TP_ONE_SCAN MAX_BS_TP_NUM_PER_SAT

#define CS8K_SAT_NL_NAME "dvbs_bs_nl"

/* cmd0 can match data0 and data1, cmd1 also can match data0 and data1 */
enum
{
    CS8K_SAT_NL_OPS_CMD0,
    //CS8K_SAT_NL_OPS_CMD1,
    __CS8K_SAT_NL_OPS_MAX,
};

#define CS8K_SAT_NL_OPS_AMOUNT (__CS8K_SAT_NL_OPS_MAX)

enum
{
    CS8K_SAT_NL_ATTR_UNSPEC,
    CS8K_SAT_NL_ATTR_DATA0,
    //CS8K_SAT_NL_ATTR_DATA1,
    __CS8K_SAT_NL_ATTR_MAX,
};

#define CS8K_SAT_NL_FAMILY_ATTR_MAX (__CS8K_SAT_NL_ATTR_MAX - 1)
#define CS8K_SAT_NL_FAMILY_ATTR_AMOUNT (__CS8K_SAT_NL_ATTR_MAX)

struct cs8k_sat_netlink_data
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

struct cs8k_sat_netlink_pid_list
{
    struct list_head list;
    struct cs8k_sat_netlink_data cs8k_sat_nl_data;
    struct net *net;
};

/* The crypto netlink socket */
//static struct sock *cs8k_sat_nlsk;

mt_fe_cs8k_sat_priv_handle g_cs8k_sat_priv = NULL;
int g_i2c_cs8k_sat = 0;
static MT_U8 cs8k_sat_notify_scan_status = 0;
mt_unf_fe_channel_info_t *cs8k_sat_pg_channel_info = NULL;

#ifdef CFG_SYNC_BLINDSCAN
/* blind scan -> found one TP -> wait user callback return */
static int event_status;
/* blindscan callback wait queue */
static wait_queue_head_t bs_cb_wq;
#endif

static int port_m88cs8k_sat_blind_scan_cancel(void *handle);
static int port_m88cs8k_sat_blind_scan_start(void *pArg);
static int port_m88cs8k_sat_blind_scan(void *handle, fe_blindscan_param_t *p_scan_info);
static int cs8k_sat_nl_list_del(void);

static LIST_HEAD(cs8k_sat_nl_pid_list);

static DEFINE_MUTEX(cs8k_sat_nl_mutex);

static struct genl_ops cs8k_sat_nl_ops[CS8K_SAT_NL_OPS_AMOUNT];

#ifdef CONFIG_NET
static struct genl_family cs8k_sat_nl_family = {
    .name = CS8K_SAT_NL_NAME,
    .version = 0x1,
    .maxattr = CS8K_SAT_NL_FAMILY_ATTR_MAX,
    .netnsok = true,
    .module = THIS_MODULE,
    .ops = cs8k_sat_nl_ops,
    .n_ops = ARRAY_SIZE(cs8k_sat_nl_ops),
};

//static struct cs8k_sat_netlink_data g_cs8k_sat_nl_data_snd = {0};

static int cs8k_sat_nl_sendmsg(mt_unf_fe_channel_info_t *p_channel_info)
{
#if 0
	struct genl_info info;
	struct cs8k_sat_netlink_data cs8k_sat_nl_data_snd;
	struct sk_buff *skb;
	struct cs8k_sat_netlink_data *cs8k_sat_nl_data;
	struct cs8k_sat_netlink_pid_list *cs8k_sat_nl_pid;
	struct cs8k_sat_netlink_pid_list *tmp;
	void *hdr;
	pid_t pid;

	if (mutex_lock_interruptible(&cs8k_sat_nl_mutex))
		return -ERESTARTSYS;

	//cs8k_sat_nl_data = &(cs8k_sat_nl_pid->cs8k_sat_nl_data);
	cs8k_sat_nl_data = &g_cs8k_sat_nl_data_snd;
	pid = cs8k_sat_nl_data->pid;
	//cs8k_sat_nl_data->cmd = 0;

	skb = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (!skb)
		return -ENOMEM;
	MT_INFO_FRONTEND("kernel sendmsg line[%d]\n",__LINE__);

	hdr = genlmsg_put(skb, 0/* from kernel */,
		cs8k_sat_nl_data->seq++, &cs8k_sat_nl_family, 0, cs8k_sat_nl_data->cmd);
	if (!hdr)
		goto out_nlmsg_free;
	MT_INFO_FRONTEND("kernel sendmsg line[%d]\n",__LINE__);

	/*
	* hdr is first nlattr or user header which size is genl_family->hdrsize,
	* if genl_family->hdrsize is 0, there is no user header
	*/

	//info.snd_portid = pid;			/* to app */
	//genl_info_net_set(&info, cs8k_sat_nl_pid->net);

	NETLINK_CB(skb).portid = 0; 	/* from kernel */

	snprintf(cs8k_sat_nl_data_snd.name, sizeof(cs8k_sat_nl_data_snd.name), "kernel nl");
	cs8k_sat_nl_data_snd.x = 1;
	cs8k_sat_nl_data_snd.y = 2;
	cs8k_sat_nl_data_snd.pid = pid;
	cs8k_sat_nl_data_snd.seq = cs8k_sat_nl_data->seq + 1;
	cs8k_sat_nl_data_snd.cmd = cs8k_sat_nl_data->cmd;
	cs8k_sat_nl_data_snd.data01 = cs8k_sat_nl_data->data01;

//		NLA_PUT_TYPE(skb, typeof(cs8k_sat_nl_data_snd), cs8k_sat_nl_data_snd.data01, cs8k_sat_nl_data_snd);
	if (nla_put(skb, cs8k_sat_nl_data_snd.data01, sizeof(cs8k_sat_nl_data_snd), &cs8k_sat_nl_data_snd))
		goto nla_put_failure;

	genlmsg_end(skb, hdr);

	//genlmsg_reply(skb, &info);

	//list_del(&(cs8k_sat_nl_pid->list));
	//kfree(cs8k_sat_nl_pid);

	mutex_unlock(&cs8k_sat_nl_mutex);

	MT_INFO_FRONTEND("cs8k_sat_nl_sendmsg line[%d]\n",__LINE__);

	return 0;

nla_put_failure:
	genlmsg_cancel(skb, hdr);
out_nlmsg_free:
	nlmsg_free(skb);
	mutex_unlock(&cs8k_sat_nl_mutex);
	return -ENOMEM;

#else

    //mt_unf_fe_channel_info_t *p_channel_info = NULL;
    struct genl_info info;
    struct cs8k_sat_netlink_data cs8k_sat_nl_data_snd;
    struct sk_buff *skb;
    struct cs8k_sat_netlink_data *cs8k_sat_nl_data;
    struct cs8k_sat_netlink_pid_list *cs8k_sat_nl_pid;
    struct cs8k_sat_netlink_pid_list *tmp;
    void *hdr;
    pid_t pid;

    if (mutex_lock_interruptible(&cs8k_sat_nl_mutex))
        return -ERESTARTSYS;

    list_for_each_entry_safe(cs8k_sat_nl_pid, tmp, &cs8k_sat_nl_pid_list, list)
    {
        cs8k_sat_nl_data = &(cs8k_sat_nl_pid->cs8k_sat_nl_data);
        pid = cs8k_sat_nl_data->pid;

        skb = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
        if (!skb)
            return -ENOMEM;

        hdr = genlmsg_put(skb, 0 /* from kernel */,
                          cs8k_sat_nl_data->seq, &cs8k_sat_nl_family, 0, cs8k_sat_nl_data->cmd);
        if (!hdr)
            goto out_nlmsg_free;

        /*
		* hdr is first nlattr or user header which size is genl_family->hdrsize,
		* if genl_family->hdrsize is 0, there is no user header
		*/

        info.snd_portid = pid; /* to app */
        genl_info_net_set(&info, cs8k_sat_nl_pid->net);

        NETLINK_CB(skb).portid = 0; /* from kernel */

        snprintf(cs8k_sat_nl_data_snd.name, sizeof(cs8k_sat_nl_data_snd.name), "kernel nl");
        cs8k_sat_nl_data_snd.freq_khz = p_channel_info->frequency; //0xaabbccdd;
        cs8k_sat_nl_data_snd.symbol_rate = p_channel_info->symbol_rate;
        cs8k_sat_nl_data_snd.port_type = p_channel_info->port_type;
        cs8k_sat_nl_data_snd.pid = pid;
        cs8k_sat_nl_data_snd.seq = cs8k_sat_nl_data->seq + 1;
        cs8k_sat_nl_data_snd.cmd = cs8k_sat_nl_data->cmd;
        cs8k_sat_nl_data_snd.data01 = cs8k_sat_nl_data->data01;

        //		NLA_PUT_TYPE(skb, typeof(cs8k_sat_nl_data_snd), cs8k_sat_nl_data_snd.data01, cs8k_sat_nl_data_snd);
        if (nla_put(skb, cs8k_sat_nl_data_snd.data01, sizeof(cs8k_sat_nl_data_snd), &cs8k_sat_nl_data_snd))
            goto nla_put_failure;

        genlmsg_end(skb, hdr);

        genlmsg_reply(skb, &info);

        list_del(&(cs8k_sat_nl_pid->list));
        kfree(cs8k_sat_nl_pid);
    }

    mutex_unlock(&cs8k_sat_nl_mutex);
    return 0;

nla_put_failure:
    genlmsg_cancel(skb, hdr);
out_nlmsg_free:
    nlmsg_free(skb);
    mutex_unlock(&cs8k_sat_nl_mutex);
    return -ENOMEM;
#endif
}
#endif

static int cs8k_sat_nl_list_del(void)
{
    struct cs8k_sat_netlink_pid_list *cs8k_sat_nl_pid;
    struct cs8k_sat_netlink_pid_list *tmp;

    if (mutex_lock_interruptible(&cs8k_sat_nl_mutex))
        return -ERESTARTSYS;

    list_for_each_entry_safe(cs8k_sat_nl_pid, tmp, &cs8k_sat_nl_pid_list, list)
    {
        list_del(&(cs8k_sat_nl_pid->list));
        kfree(cs8k_sat_nl_pid);
    }

    mutex_unlock(&cs8k_sat_nl_mutex);
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

mt_unf_fe_fec_type_t port_m88cs8k_sat_deparse_dvb_type(MT_FE_TYPE dvb_type)
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

mt_unf_fe_fecrate_t port_m88cs8k_sat_deparse_code_rate(MT_FE_CODE_RATE code_rate)
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

#if 0
/* Convert downlink frequency to IF, calculate LNB 22K status synchronously, for connect */
/*Downlink freq dominates the course, i.e, the freq band is decided by the downlink freq*/
static void port_m88cs8k_sat_downlink_freq_to_if(void *handle, mt_unf_fe_lnb_config_t* p_lnb_config,
                                      mt_unf_fe_polar_t polar, mt_u32 downlink_freq,
                                      mt_u32* p_if, mt_unf_fe_lnb_22k_t *p_lnb_22k)
{
    /* Default */
    if (MT_NULL != p_lnb_22k)
    {
        *p_lnb_22k = MT_UNF_FE_LNB_22K_OFF;
    }

    if ((SAT_C_MIN_KHZ <= downlink_freq)
            && (SAT_C_MAX_KHZ >= downlink_freq))
    {
        p_lnb_config->lnb_band = MT_UNF_FE_LNB_BAND_C;
    }
    else if ((SAT_KU_MIN_KHZ <= downlink_freq)
            && (SAT_KU_MAX_KHZ >= downlink_freq))
    {
        p_lnb_config->lnb_band = MT_UNF_FE_LNB_BAND_KU;
    }
    else
    {
        MT_ERR_FE("Error freq!\n");
        return;
    }

    switch (p_lnb_config->lnb_band)
    {
        /* C band, IF = LO - downlink frequency */
    case MT_UNF_FE_LNB_BAND_C:
        /* Single LO */
        if ((MT_UNF_FE_LNB_SINGLE_FREQUENCY == p_lnb_config->lnb_type)
           || (p_lnb_config->high_lo == p_lnb_config->low_lo))
        {
            *p_if = p_lnb_config->low_lo * 1000 - downlink_freq;
        }
        /* Dual LO */
        else
        {
            /* V/R polarization, use high LO */
            if ((MT_UNF_FE_POLARIZATION_V == polar) || (MT_UNF_FE_POLARIZATION_R == polar))
            {
                *p_if = p_lnb_config->high_lo * 1000 - downlink_freq;
            }
            /* H/L polarization, use low LO */
            else
            {
                *p_if = p_lnb_config->low_lo * 1000 - downlink_freq;
            }
        }

        break;

    /* Ku band, IF = downlink frequency - LO */
    case MT_UNF_FE_LNB_BAND_KU:

        /* Single LO */
        if ((MT_UNF_FE_LNB_SINGLE_FREQUENCY == p_lnb_config->lnb_type)
           || (p_lnb_config->high_lo == p_lnb_config->low_lo))
        {
            *p_if = downlink_freq - p_lnb_config->low_lo * 1000;
        }
        /* Dual LO */
        else
        {
            /* downlink frequency >= 11700MHz, use high LO */
            if ((downlink_freq >= SAT_DOWNLINK_FREQ_KU_MID * 1000))
            {
                *p_if = downlink_freq - p_lnb_config->high_lo * 1000;

                /* Ku dual LO LNB use 22K select high LO */
                if (MT_NULL != p_lnb_22k)
                {
                    *p_lnb_22k = MT_UNF_FE_LNB_22K_ON;
                }
            }
            /* downlink frequency < 11700MHz, use low LO */
            else
            {
                *p_if = downlink_freq - p_lnb_config->low_lo * 1000;
            }
        }

        break;

    default:
        break;
    }
}

//#else
/* Convert downlink frequency to IF, calculate LNB 22K status synchronously, for connect */
/*Downlink freq dominates the course, i.e, the freq band is decided by the downlink freq*/
static void port_m88cs8k_sat_downlink_freq_to_if(void *handle, mt_unf_fe_lnb_config_t* p_lnb_config,
                                      mt_unf_fe_polar_t polar, mt_u32 downlink_freq)
{
    mt_fe_cs8k_sat_priv_handle p_priv = (mt_fe_cs8k_sat_priv_handle)handle;
	MT_FE_CS8000_SAT_Device_Handle cs8k_handle = p_priv->cs8k_handle;

	/* Default */
    if (MT_NULL != p_lnb_22k)
    {
        *p_lnb_22k = MT_UNF_FE_LNB_22K_OFF;
    }

    if ((SAT_C_MIN_KHZ <= downlink_freq)
            && (SAT_C_MAX_KHZ >= downlink_freq))
    {
        cs8k_handle->lnb_cfg.b = MT_UNF_FE_LNB_BAND_C;
    }
    else if ((SAT_KU_MIN_KHZ <= downlink_freq)
            && (SAT_KU_MAX_KHZ >= downlink_freq))
    {
        p_lnb_config->lnb_band = MT_UNF_FE_LNB_BAND_KU;
    }
    else
    {
        MT_ERR_FE("Error freq!\n");
        return;
    }

    switch (p_lnb_config->lnb_band)
    {
        /* C band, IF = LO - downlink frequency */
    case MT_UNF_FE_LNB_BAND_C:
        /* Single LO */
        if ((MT_UNF_FE_LNB_SINGLE_FREQUENCY == p_lnb_config->lnb_type)
           || (p_lnb_config->high_lo == p_lnb_config->low_lo))
        {
            *p_if = p_lnb_config->low_lo * 1000 - downlink_freq;
        }
        /* Dual LO */
        else
        {
            /* V/R polarization, use high LO */
            if ((MT_UNF_FE_POLARIZATION_V == polar) || (MT_UNF_FE_POLARIZATION_R == polar))
            {
                *p_if = p_lnb_config->high_lo * 1000 - downlink_freq;
            }
            /* H/L polarization, use low LO */
            else
            {
                *p_if = p_lnb_config->low_lo * 1000 - downlink_freq;
            }
        }

        break;

    /* Ku band, IF = downlink frequency - LO */
    case MT_UNF_FE_LNB_BAND_KU:

        /* Single LO */
        if ((MT_UNF_FE_LNB_SINGLE_FREQUENCY == p_lnb_config->lnb_type)
           || (p_lnb_config->high_lo == p_lnb_config->low_lo))
        {
            *p_if = downlink_freq - p_lnb_config->low_lo * 1000;
        }
        /* Dual LO */
        else
        {
            /* downlink frequency >= 11700MHz, use high LO */
            if ((downlink_freq >= SAT_DOWNLINK_FREQ_KU_MID * 1000))
            {
                *p_if = downlink_freq - p_lnb_config->high_lo * 1000;

                /* Ku dual LO LNB use 22K select high LO */
                if (MT_NULL != p_lnb_22k)
                {
                    *p_lnb_22k = MT_UNF_FE_LNB_22K_ON;
                }
            }
            /* downlink frequency < 11700MHz, use low LO */
            else
            {
                *p_if = downlink_freq - p_lnb_config->low_lo * 1000;
            }
        }

        break;

    default:
        break;
    }
}

#endif

static int port_m88cs8k_sat_channel_set(void *handle, mt_unf_fe_connect_para_t *para)
{
    //MT_U32 for_scan = 0;
    MT_FE_RET ret = 0;
    MT_FE_TYPE dvbs_type = MtFeType_Undef;
    mt_unf_fe_channel_info_t *p_channel_info = NULL;
    mt_fe_cs8k_sat_priv_handle p_priv = (mt_fe_cs8k_sat_priv_handle)handle;
    MT_FE_CS8000_SAT_Device_Handle cs8k_handle = p_priv->cs8k_handle;

    p_channel_info = &(para->channel_info);
    para->channel_info.lock = 0;

    if (MT_UNF_PORT_TYPE_DVBS == para->connect_param.sat.port_type)
    {
        //cs8k_handle->demod_type = MtFeType_DvbS;
        dvbs_type = MtFeType_DvbS;
    }
    else if (MT_UNF_PORT_TYPE_DVBS2 == para->connect_param.sat.port_type)
    {
        //cs8k_handle->demod_type = MtFeType_DvbS2;
        dvbs_type = MtFeType_DvbS2;
    }
    else
    {
        //cs8k_handle->demod_type = MtFeType_DvbS2;
        dvbs_type = MtFeType_DTV_Unknown;
    }

    MT_INFO_FRONTEND("cs8k_channel set: feq %d, sym = %d, bs = %d, type = %d, use_uc = %d\n",
                     para->connect_param.sat.freq,
                     para->connect_param.sat.sym_rate,
                     p_priv->for_scan,
                     para->connect_param.sat.port_type,
                     para->connect_param.sat.uc_param.use_uc);

    if (para->connect_param.sat.uc_param.use_uc)
    {
        cs8k_handle->lnb_cfg.bUnicable = 1;
        cs8k_handle->lnb_cfg.iBankIndex = para->connect_param.sat.uc_param.bank;
        cs8k_handle->lnb_cfg.iUBIndex = para->connect_param.sat.uc_param.user_band;
        cs8k_handle->lnb_cfg.iUBFreqMHz = para->connect_param.sat.uc_param.ub_freq_mhz;
    }
    else
    {
        cs8k_handle->lnb_cfg.bUnicable = 0;
    }

    memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));
    //priv->for_scan = p_channel_set_info->for_scan;

    ret = mt_fe_dmd_cs8k_sat_connect(cs8k_handle, para->connect_param.sat.freq / 1000,
                                     para->connect_param.sat.sym_rate,
                                     dvbs_type,
                                     p_priv->for_scan);

    if (ret == MtFeErr_Ok)
    {
        /* set check lock delay time */
        if (dvbs_type == MtFeType_DvbS)
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
        else if (dvbs_type == MtFeType_DvbS2)
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
        //MT_INFO_FRONTEND("[%s] line:%d lock_time %d\n", __func__, __LINE__, para->channel_set_info.lock_time);

        return MT_SUCCESS;
    }
    else
    {
        MT_ERR_FRONTEND("mt_fe_dmd_cs8000_connect failed %d\n", ret);
    }

    return MT_FAILURE;
}

static void port_m88cs8k_sat_set_22k_onoff(void *handle, MT_U8 onoff_22k, MT_U8 diseqc_out_when_lnb_off)
{
    U8 val_0xa1, val_0xa2;
    mt_fe_cs8k_sat_priv_handle p_priv = (mt_fe_cs8k_sat_priv_handle)handle;
    MT_FE_CS8000_SAT_Device_Handle cs8k_handle = p_priv->cs8k_handle;
    //pinmux_configure();

    _mt_fe_cs8k_sat_dmd_get_reg(cs8k_handle, 0xa1, &val_0xa1);
    _mt_fe_cs8k_sat_dmd_get_reg(cs8k_handle, 0xa2, &val_0xa2);

    if (onoff_22k == MtFe_True)
    {
        val_0xa1 |= 0x04;
        val_0xa1 &= ~0x03;
        val_0xa1 &= ~0x40;
        val_0xa2 &= ~0xc0;
    }
    else
    {
        if (diseqc_out_when_lnb_off == MT_FE_CS8K_SAT_PIN_LEVEL_HIGH)
        {
            val_0xa2 |= 0xc0;
        }
        else
        {
            val_0xa2 &= ~0xc0;
            val_0xa2 |= 0x80;
        }
    }

    _mt_fe_cs8k_sat_dmd_set_reg(cs8k_handle, 0xa2, val_0xa2);
    _mt_fe_cs8k_sat_dmd_set_reg(cs8k_handle, 0xa1, val_0xa1);

    MT_INFO_FRONTEND("port_m88cs8k_sat_set_22k_onoff %d\n", onoff_22k);
}

static void port_m88cs8k_sat_set_lnb_voltage(void *handle, MT_FE_LNB_VOLTAGE voltage, MT_U8 vsel_pin_13v)
{
    MT_U8 val_0xa2;
    // fix Bug 100042 start
    MT_U8 buf[2];
    MT_U8 i2c_id, slave_addr;
    mt_fe_cs8k_sat_priv_handle p_priv = (mt_fe_cs8k_sat_priv_handle)handle;
    MT_FE_CS8000_SAT_Device_Handle cs8k_handle = p_priv->cs8k_handle;
    //pinmux_configure();

    if (p_priv->lnbctrl_dev == MT_UNF_LNBCTRL_DEV_TYPE_A8304)
    {

        i2c_id = p_priv->lnb_i2c_id;
        slave_addr = p_priv->lnb_dev_addr;

        buf[0] = 0;
        if (voltage == MtFeLNB_13V)
        {
            buf[1] = 0x32; //0x35; // fix bug 100134   // original val = 0x32;
        }
        else if (voltage == MtFeLNB_18V)
        {
            buf[1] = 0x3b; //0x3d; // fix bug 100134   // original val = 0x3b;
        }
        //i2c_id, sla_addr, buffer, len, param
        _mt_fe_cs8k_sat_lnb_write(i2c_id, slave_addr, buf, 2);

        _mt_fe_cs8k_sat_lnb_write(i2c_id, slave_addr, buf, 1);
        //i2c_id, sla_addr, buffer, len

        _mt_fe_cs8k_sat_lnb_read(i2c_id, slave_addr, buf, 1);
        buf[0] = 0x00;
        _mt_fe_cs8k_sat_lnb_write(i2c_id, slave_addr, buf, 2);
        //MT_INFO_FRONTEND("port_m88cs8k_sat_set_lnb_voltage\n");
        return;
    }
    // fix Bug 100042 end

    _mt_fe_cs8k_sat_dmd_get_reg(cs8k_handle, 0xa2, &val_0xa2);
    MT_INFO_FRONTEND("port_m88cs8k_sat_set_lnb_voltage %d\n", voltage);

    if (vsel_pin_13v == MT_FE_CS8K_SAT_PIN_LEVEL_HIGH)
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

    _mt_fe_cs8k_sat_dmd_set_reg(cs8k_handle, 0xa2, val_0xa2);
    val_0xa2 = 0;
    _mt_fe_cs8k_sat_dmd_get_reg(cs8k_handle, 0xa2, &val_0xa2);
}

static void port_m88cs8k_sat_set_lnb_onoff(void *handle, MT_U8 lnb_enable, mt_unf_fe_pin_config_para_t *pin_config)
{
    U8 val_0xa1, val_0xa2; //, pin, level, value;
    MT_U8 i2c_id, slave_addr;
    MT_U8 buf[2];
    mt_fe_cs8k_sat_priv_handle p_priv = (mt_fe_cs8k_sat_priv_handle)handle;
    MT_FE_CS8000_SAT_Device_Handle cs8k_handle = p_priv->cs8k_handle;
    //pinmux_configure();
    // fix Bug 100042 start

    if (p_priv->lnbctrl_dev == MT_UNF_LNBCTRL_DEV_TYPE_A8304)
    {
        i2c_id = p_priv->lnb_i2c_id;
        slave_addr = p_priv->lnb_dev_addr;

        buf[0] = 0;
        if (lnb_enable == MtFe_True)
        {
            if (p_priv->lnb_polar == PORT_PORLAR_HORIZONTAL)
            {
                buf[1] = 0x3b; //0x3d; // fix bug 100134   // original val = 0x3b;
            }
            else
            {
                buf[1] = 0x32; //0x35; // fix bug 100134   // original val = 0x32;
            }
        }
        else
        {
            buf[1] = 0x20;
        }
        _mt_fe_cs8k_sat_lnb_write(i2c_id, slave_addr, buf, 2);
        _mt_fe_cs8k_sat_lnb_write(i2c_id, slave_addr, buf, 1);
        _mt_fe_cs8k_sat_lnb_read(i2c_id, slave_addr, buf, 1);
        buf[0] = 0;
        _mt_fe_cs8k_sat_lnb_write(i2c_id, slave_addr, buf, 2);
        //MT_INFO_FRONTEND("port_m88cs8k_sat_set_lnb_onoff\n");
        return;
    }

    _mt_fe_cs8k_sat_dmd_get_reg(cs8k_handle, 0xa1, &val_0xa1);
    _mt_fe_cs8k_sat_dmd_get_reg(cs8k_handle, 0xa2, &val_0xa2);

    MT_INFO_FRONTEND("port_m88cs8k_sat_set_lnb_onoff %d\n", lnb_enable);

    if (lnb_enable != MtFe_True) // off
    {
        val_0xa1 |= 0x40;
        if (pin_config->lnb_enable_by_mcu == 0)
        {
            /*set LNB_EN pin HIGH or LOW */
            if (pin_config->lnb_enable == MT_FE_CS8K_SAT_PIN_LEVEL_HIGH)
            {
                val_0xa2 &= ~0x02;
            }
            else
            {
                val_0xa2 |= 0x02;
            }
            //MT_INFO_FRONTEND(" port_m88cs8k_sat_set_lnb_onoff line:%d\n", __LINE__);
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
				MT_INFO_FRONTEND("off: set pin output\n");
			}
#if 1
			gpio_get_value(pin,&value);
			MT_INFO_FRONTEND("off: get lnb_enable_pin value[%d]\n", value);
			if (value == level)
			{
				gpio_set_value(pin,(level == GPIO_LEVEL_HIGH) ? GPIO_LEVEL_LOW : GPIO_LEVEL_HIGH);
				MT_INFO_FRONTEND("off: set lnb_enable_pin value[%d]\n", (level == GPIO_LEVEL_HIGH) ? GPIO_LEVEL_LOW : GPIO_LEVEL_HIGH);
			}
#else
			gpio_set_value(pin,(level == GPIO_LEVEL_HIGH) ? GPIO_LEVEL_LOW : GPIO_LEVEL_HIGH);
#endif
#endif
        }

        /*set V_SEL Pin mode*/
        if (pin_config->vsel_when_lnb_off == MT_FE_CS8K_SAT_PIN_LEVEL_HIGH)
        {
            val_0xa2 |= 0x01;
        }
        else
        {
            val_0xa2 &= ~0x01;
        }
        /*set DiseQc_OUT mode*/
        if (pin_config->diseqc_out_when_lnb_off == MT_FE_CS8K_SAT_PIN_LEVEL_HIGH)
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
            //MT_INFO_FRONTEND("port_m88cs8k_sat_set_lnb_onoff line:%d.\n", __LINE__);
            if (pin_config->lnb_enable == MT_FE_CS8K_SAT_PIN_LEVEL_HIGH)
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
			MT_INFO_FRONTEND("lnb_enable_pin[%d], level[%d]\n", pin, level);
			//hal_pinmux_gpio_enable(pin, TRUE);
			//MT_INFO_FRONTEND("gpio_ioctl\n");
			//gpio_ioctl(GPIO_CMD_IO_ENABLE, pin, TRUE);
			MT_INFO_FRONTEND("gpio_get_dir\n");
			gpio_get_dir(pin, &value);
			if (GPIO_DIR_INPUT == value)
			{
			  MT_INFO_FRONTEND("set lnb_enable_pin output\n");
			  gpio_set_dir(pin, GPIO_DIR_OUTPUT);
			}
#if 1
			gpio_get_value(pin, &value);
			MT_INFO_FRONTEND("off: get lnb_enable_pin value[%d]\n", value);
			if (value != level)
			{
				MT_INFO_FRONTEND("set lnb_enable_pin value[%d]\n", level);
				gpio_set_value(pin, level);
			}
#else
		gpio_set_value(pin, level);
#endif
#endif
        }
    }

    _mt_fe_cs8k_sat_dmd_set_reg(cs8k_handle, 0xa2, val_0xa2);
    _mt_fe_cs8k_sat_dmd_set_reg(cs8k_handle, 0xa1, val_0xa1);
}

static int port_m88cs8k_sat_diseqc_sendmsg(void *handle, mt_unf_fe_diseqc_sendmsg_t *p_diseqc_sendmsg)
{
    mt_fe_cs8k_sat_priv_handle p_priv = (mt_fe_cs8k_sat_priv_handle)handle;
    MT_FE_CS8000_SAT_Device_Handle cs8k_handle = p_priv->cs8k_handle;
    MT_FE_DiSEqC_MSG msg;
    MT_FE_RET ret = MtFeErr_Undef;

    //return MT_SUCCESS;

    if (p_diseqc_sendmsg->len == 0) // || p_diseqc_sendmsg->p_tx_buf == NULL)
    {
        return MT_FAILURE;
    }
    if (p_diseqc_sendmsg->len > 8)
    {
        MT_ERR_FRONTEND("ERROR:p_diseqc_sendmsg->len > 8\n");
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
        port_m88cs8k_sat_set_22k_onoff(handle, 0, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);

        /* make sure the voltage stable */
        //mdelay(MT_FE_CS8K_SAT_DISEQC_COMMAND_START_DELAY);
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

    MT_INFO_FRONTEND("DiSEqC send msg[0x%x, 0x%x, 0x%x, 0x%x], send size[%d]\n",
                     msg.data_send[0],
                     msg.data_send[1],
                     msg.data_send[2],
                     msg.data_send[3],
                     msg.size_send);

    ret = mt_fe_dmd_cs8k_sat_DiSEqC_send_msg(cs8k_handle, &msg);

    if (p_priv->onoff_22k != 0)
    {
        /* make sure the voltage stable */
        //mdelay(MT_FE_CS8K_SAT_DISEQC_COMMAND_END_DELAY);
        usleep_range(15 * 1000, 20 * 1000); // delay 15~20ms

        /* 22k was closed, open it */
        port_m88cs8k_sat_set_22k_onoff(handle, 1, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);
    }

    return (ret == MtFeErr_Ok) ? MT_SUCCESS : MT_FAILURE;
}

static int port_m88cs8k_sat_diseqc_send_tone_burst(void *handle, mt_u8 mode)
{
    mt_fe_cs8k_sat_priv_handle p_priv = (mt_fe_cs8k_sat_priv_handle)handle;
    MT_FE_CS8000_SAT_Device_Handle cs8k_handle = p_priv->cs8k_handle;
    MT_FE_RET ret = MtFeErr_Undef;

    p_priv->cur_diseqc.mode = 0; //PORT_DISEQC_BURST0;//p_diseqc_cmd->mode;

    if (p_priv->lnb_onoff == 0)
    {
        return MT_SUCCESS;
    }

    if (p_priv->onoff_22k != 0)
    {
        /* 22k is opened, close it */
        port_m88cs8k_sat_set_22k_onoff(handle, 0, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);

        /* make sure the voltage stable */
        //mdelay(MT_FE_CS8K_SAT_DISEQC_COMMAND_START_DELAY);
        usleep_range(15 * 1000, 20 * 1000); // delay 15~20ms
    }

    mode = (mode == 0) ? MtFeDiSEqCToneBurst_Unmoulated : MtFeDiSEqCToneBurst_Moulated;
    ret = mt_fe_dmd_cs8k_sat_DiSEqC_send_tone_burst(cs8k_handle, mode, 0);

    if (p_priv->onoff_22k != 0)
    {
        /* make sure the voltage stable */
        //mdelay(MT_FE_CS8K_SAT_DISEQC_COMMAND_END_DELAY);
        usleep_range(15 * 1000, 20 * 1000); // delay 15~20ms

        /* 22k was closed, open it */
        port_m88cs8k_sat_set_22k_onoff(handle, 1, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);
    }

    return (ret == MtFeErr_Ok) ? MT_SUCCESS : MT_FAILURE;
}

static int port_m88cs8k_sat_diseqc_recvmsg(void *handle, mt_unf_fe_diseqc_recvmsg_t *p_diseqc_recvmsg)
{
    mt_fe_cs8k_sat_priv_handle p_priv = (mt_fe_cs8k_sat_priv_handle)handle;
    MT_FE_CS8000_SAT_Device_Handle cs8k_handle = p_priv->cs8k_handle;

    MT_FE_DiSEqC_MSG msg;
    //MT_FE_DiSEqC_TONE_BURST mode;
    MT_FE_RET ret = MtFeErr_Undef;

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
        port_m88cs8k_sat_set_22k_onoff(handle, 0, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);

        /* make sure the voltage stable */
        //mdelay(MT_FE_CS8K_SAT_DISEQC_COMMAND_START_DELAY);
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

    ret = mt_fe_dmd_cs8k_sat_DiSEqC_receive_msg(cs8k_handle, &msg);
    MT_INFO_FRONTEND("DiSEqC recv msg[0x%x, 0x%x, 0x%x, 0x%x] recv size[%d]\n",
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
        //mdelay(MT_FE_CS8K_SAT_DISEQC_COMMAND_START_DELAY);
        usleep_range(15 * 1000, 20 * 1000); // delay 15~20ms

        /* 22k was colsed, open it */
        port_m88cs8k_sat_set_22k_onoff(handle, 1, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);
    }

    return (ret == MtFeErr_Ok) ? MT_SUCCESS : MT_FAILURE;
}

static int port_m88cs8k_sat_get_status(void *handle, mt_unf_fe_status_t *p_status)
{
    mt_fe_cs8k_sat_priv_handle p_priv = (mt_fe_cs8k_sat_priv_handle)handle;
    MT_FE_CS8000_SAT_Device_Handle cs8k_handle = p_priv->cs8k_handle;
    MT_FE_LOCK_STATE stat = 0;

    mt_fe_dmd_cs8k_sat_get_lock_state(cs8k_handle, &stat, p_priv->for_scan);

    if (stat == MtFeLockState_Locked)
        p_status->lock_status = MT_UNF_FE_SIGNAL_LOCKED;
    else
    {
        p_status->lock_status = MT_UNF_FE_SIGNAL_DROPPED;
        if (stat == MtFeLockState_Unlocked)
            p_status->unlock_reason = MT_UNF_FE_STATE_UNLOCKED;
        else
            p_status->unlock_reason = MT_UNF_FE_STATE_WAITING;
    }

    return MT_SUCCESS;
}

static int port_m88cs8k_sat_get_signal_quality(void *handle, MT_U32 *p_quality)
{
    mt_fe_cs8k_sat_priv_handle p_priv = (mt_fe_cs8k_sat_priv_handle)handle;
    MT_FE_CS8000_SAT_Device_Handle cs8k_handle = p_priv->cs8k_handle;
    int ret = 0;
    U8 percent = 0;

    ret = mt_fe_dmd_cs8k_sat_get_sat_quality(cs8k_handle, &percent);
    if (ret < 0)
        return MT_FAILURE;

    *p_quality = percent;

    return MT_SUCCESS;
}

static int port_m88cs8k_sat_get_ber(void *handle, MT_U32 *p_ber)
{
    mt_fe_cs8k_sat_priv_handle p_priv = (mt_fe_cs8k_sat_priv_handle)handle;
    MT_FE_CS8000_SAT_Device_Handle cs8k_handle = p_priv->cs8k_handle;
    //U32 ber0 = 0;
    //U32 ber1 = 0;
    //U32 ber2 = 0;
    U32 err_packages = 0;
    U32 total_packages = 0;
    //U32 max = 20;
    //int ret = 0;

#if 0

    ret = mt_fe_dmd_cs8k_sat_get_ber(handle, &errbits, &errtot);
    if(ret < 0)
        return ret;
    while(max --)
    {
        if((errbits / errtot) == 0)
        {
            errbits *= 10;
            ber2 ++;
        }
        else
        {
            ber0 = errbits / errtot;
            ber1 = ((errbits - (492ber0 * errtot)) * 1000) / errtot;
            break;
        }
    }
#endif

    mt_fe_dmd_cs8k_sat_get_per(cs8k_handle, &total_packages, &err_packages);

    p_ber[0] = total_packages;
    p_ber[1] = err_packages;
    p_ber[2] = 0;

    return MT_SUCCESS;
}

static int port_m88cs8k_sat_get_snr(void *handle, MT_U32 *p_snr)
{
    mt_fe_cs8k_sat_priv_handle p_priv = (mt_fe_cs8k_sat_priv_handle)handle;
    MT_FE_CS8000_SAT_Device_Handle cs8k_handle = p_priv->cs8k_handle;
    U8 _snr = 0;
    int ret = 0;

    ret = mt_fe_dmd_cs8k_sat_get_snr(cs8k_handle, &_snr);
    if (ret < 0)
        return MT_FAILURE;

    *p_snr = _snr;

    return MT_SUCCESS;
}

static int port_m88cs8k_sat_get_signal_strength(void *handle, MT_U32 *p_strength)
{
    mt_fe_cs8k_sat_priv_handle p_priv = (mt_fe_cs8k_sat_priv_handle)handle;
    MT_FE_CS8000_SAT_Device_Handle cs8k_handle = p_priv->cs8k_handle;
    U8 _strength = 0;
    int ret = 0;

    ret = mt_fe_dmd_cs8k_sat_get_strength(cs8k_handle, &_strength);
    if (ret < 0)
        return MT_FAILURE;

    *p_strength = _strength;

    return MT_SUCCESS;
}

static void port_m88cs8k_sat_get_signal_info(void *handle, mt_unf_fe_signal_info_t *p_sig_info)
{
    mt_fe_cs8k_sat_priv_handle p_priv = (mt_fe_cs8k_sat_priv_handle)handle;
    MT_FE_CS8000_SAT_Device_Handle cs8k_handle = p_priv->cs8k_handle;

    if (cs8k_handle->tp_cfg.mConnectType == MtFeType_DvbS)
        p_sig_info->sig_info.sat.sat_type = MT_UNF_FE_DVBS;
    else if (cs8k_handle->tp_cfg.mConnectType == MtFeType_DvbS2)
        p_sig_info->sig_info.sat.sat_type = MT_UNF_FE_DVBS2;
    else
        p_sig_info->sig_info.sat.sat_type = MT_UNF_FE_BUTT;

    /*
	p_sig_info->frequency = handle->m_device_ctt2.input_params.input_freq_kHz;
	p_sig_info->bandwidth = handle->m_device_ctt2.input_params.demod_bandwidth;
	*/
}

static int port_m88cs8k_sat_get_signal_agc(void *handle, mt_u32 *p_agc)
{
    mt_fe_cs8k_sat_priv_handle p_priv = (mt_fe_cs8k_sat_priv_handle)handle;
    MT_FE_CS8000_SAT_Device_Handle cs8k_handle = p_priv->cs8k_handle;
    int ret = 0;
    mt_u32 tuner_gain = 0;
    mt_s32 tuner_strength = 0;

    if (cs8k_handle->tuner_cfg.tuner_get_strength)
    {
        ret = cs8k_handle->tuner_cfg.tuner_get_strength(cs8k_handle, &tuner_gain, &tuner_strength);
        if (ret < 0)
            return MT_FAILURE;
        p_agc[0] = tuner_gain;
        MT_INFO_FRONTEND("drv tuner_gain[%d]\n", tuner_gain);
    }
    else
        p_agc[0] = 0;

    return MT_SUCCESS;
}

static int port_m88cs8k_sat_standby(void *handle)
{
    mt_fe_cs8k_sat_priv_handle p_priv = (mt_fe_cs8k_sat_priv_handle)handle;
    MT_FE_CS8000_SAT_Device_Handle cs8k_handle = p_priv->cs8k_handle;

    mt_fe_dmd_cs8k_sat_sleep(cs8k_handle);

    if (cs8k_handle->tuner_cfg.tuner_sleep != NULL)
    {
        cs8k_handle->tuner_cfg.tuner_sleep(handle);
    }

    return MT_SUCCESS;
}

static int port_m88cs8k_sat_wakeup(void *handle)
{
    mt_fe_cs8k_sat_priv_handle p_priv = (mt_fe_cs8k_sat_priv_handle)handle;
    MT_FE_CS8000_SAT_Device_Handle cs8k_handle = p_priv->cs8k_handle;

    mt_fe_dmd_cs8k_sat_wake_up(cs8k_handle);

    if (cs8k_handle->tuner_cfg.tuner_wakeup != NULL)
    {
        cs8k_handle->tuner_cfg.tuner_wakeup(handle);
    }

    return MT_SUCCESS;
}

static int port_m88cs8k_sat_get_default_timeout(void *handle, mt_u32 *timeout)
{
    *timeout = 120;
    return MT_SUCCESS;
}

static int port_m88cs8k_sat_set_io(void *handle, MT_BOOL onoff)
{
#if 0
    int ret = 0;
    ret = mt_fe_dmd_ca8k_cab_set_io(handle, onoff);
    if(ret < 0)

        return ret;
#endif
    return MT_SUCCESS;
}

static int port_m88cs8k_sat_channel_connect(void *handle, mt_unf_fe_connect_para_t *para)
{
    mt_unf_fe_channel_info_t *p_channel_info = NULL;
    mt_fe_cs8k_sat_priv_handle p_priv = (mt_fe_cs8k_sat_priv_handle)handle;
    MT_FE_CS8000_SAT_Device_Handle cs8k_handle = p_priv->cs8k_handle;

#ifdef FOR_PORT_CS8K_SAT_CONNECT_SYCHRONOUS
    MT_FE_RET ret = MtFeErr_Ok;
    MT_FE_LOCK_STATE status = 0;
    MT_U32 cnt = 0;
#endif

    MT_U8 for_bs = 0;
    MT_FE_TYPE dvbs_type = MtFeType_Undef;
    MT_FE_BS_TP_INFO bs_tpinfo;
    MT_FE_TP_INFO tp_info;
    MT_U8 voltage = 0;

    p_channel_info = &(para->channel_info);
    //pinmux_configure();

    if (MT_UNF_PORT_TYPE_DVBS == para->connect_param.sat.port_type)
    {
        dvbs_type = MtFeType_DvbS;
    }
    else if (MT_UNF_PORT_TYPE_DVBS2 == para->connect_param.sat.port_type)
    {
        dvbs_type = MtFeType_DvbS2;
    }
    else
    {
        dvbs_type = MtFeType_DTV_Unknown;
    }

    MT_INFO_FRONTEND("cs8k_sat connect: feq %d, sym = %d, bs = %d, type = %d, use_uc = %d\n",
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
        port_m88cs8k_sat_set_lnb_voltage(handle, voltage, p_priv->cfg.pin_config.vsel_when_13v);
        p_priv->lnb_polar = (MT_U8)para->connect_param.sat.polarization;
        p_priv->lnb_voltage = voltage;
    }

    if (para->connect_param.sat.uc_param.use_uc)
    {
        cs8k_handle->lnb_cfg.bUnicable = 1;
        cs8k_handle->lnb_cfg.iBankIndex = para->connect_param.sat.uc_param.bank;
        cs8k_handle->lnb_cfg.iUBIndex = para->connect_param.sat.uc_param.user_band;
        cs8k_handle->lnb_cfg.iUBFreqMHz = para->connect_param.sat.uc_param.ub_freq_mhz;
        MT_INFO_FRONTEND("drv bank[%d] user_band[%d] ub_freq_mhz[%d]\n",
                         cs8k_handle->lnb_cfg.iBankIndex,
                         cs8k_handle->lnb_cfg.iUBIndex,
                         cs8k_handle->lnb_cfg.iUBFreqMHz);
    }
    else
    {
        cs8k_handle->lnb_cfg.bUnicable = 0;
    }

    memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));
    para->channel_info.lock = 0;

    if (for_bs)
    {
        p_priv->for_scan = 1;
        bs_tpinfo.tp_num = 0;
        bs_tpinfo.p_tp_info = &tp_info;
        bs_tpinfo.p_tp_info[0].dvb_type = dvbs_type;
        bs_tpinfo.p_tp_info[0].freq_KHz = p_channel_info->frequency;
        bs_tpinfo.p_tp_info[0].sym_rate_KSs = para->connect_param.sat.sym_rate;

#ifdef OLD_BLINDSCAN_ALGO
        if (_mt_fe_dmd_cs8k_sat_bs_connect(p_priv->cs8k_handle, &bs_tpinfo, 0, 1) == MtFeErr_Ok)
#else
        if (_mt_fe_dmd_cs8k_sat_bs_connect_test(p_priv->cs8k_handle, &bs_tpinfo) == MtFeErr_Ok)
#endif
        {
            if (bs_tpinfo.tp_num > 0)
            {
                p_channel_info->frequency = bs_tpinfo.p_tp_info[0].freq_KHz;
                p_channel_info->symbol_rate = bs_tpinfo.p_tp_info[0].sym_rate_KSs;
                p_channel_info->port_type = port_m88cs8k_sat_deparse_dvb_type(bs_tpinfo.p_tp_info[0].dvb_type);
                p_channel_info->fec_inner = port_m88cs8k_sat_deparse_code_rate(bs_tpinfo.p_tp_info[0].code_rate);
                p_channel_info->lock = 1;
            }
            return MT_SUCCESS;
        }
    }
    else
    {
        p_priv->for_scan = 0;
        para->channel_set_info.for_scan = 0;
        port_m88cs8k_sat_channel_set(p_priv, para);

#ifdef FOR_PORT_CS8K_SAT_CONNECT_SYCHRONOUS
        mt_u32 curTime;
        //mdelay(50);
        msleep(50);

        if (ret == MT_SUCCESS)
        {
            //MT_INFO_FRONTEND("lock_timeout_ms :%d\n", para->stChannelSetInfo.lock_time);
            for (cnt = 0; cnt < para->channel_set_info.lock_time; cnt += 10)
            {
                mt_fe_dmd_cs8k_sat_get_lock_state(cs8k_handle, &status, for_bs);
                para->channel_info.lock = (status == MtFeLockState_Locked) ? 1 : 0;
                if (para->channel_info.lock)
                {
                    //MT_INFO_FRONTEND("cs8k_sat connect lock success\n");
                    return MT_SUCCESS;
                }
                //mdelay(10);
                msleep(10);
                mt_drv_sys_gettimestampms(&curTime);
                if (curTime - p_priv->lock_or_connect_time > para->channel_set_info.lock_time)
                {
                    if (cnt > para->channel_set_info.lock_time / 2)
                    {
                        cnt = para->channel_set_info.lock_time;
                    }
                }
            }
        }
        return ERR_TIMEOUT;
#else
        //MT_INFO_FRONTEND("[%s] line:%d lock_timeout_ms %d lock.\n", __func__, __LINE__, para->stChannelSetInfo.lock_time);
        return MT_SUCCESS;
#endif
    }
    return MT_FAILURE;
}

//static int port_m88cs8k_sat_ioctl(void *handle, mt_u32 cmd, mt_u32 param)
static int port_m88cs8k_sat_ioctl(void *handle, mt_u32 cmd, ulong param)
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

    mt_fe_cs8k_sat_priv_handle p_priv = (mt_fe_cs8k_sat_priv_handle)handle;
    MT_FE_CS8000_SAT_Device_Handle cs8k_handle;

    if (p_priv == NULL)
    {
        return MT_FAILURE;
    }

    cs8k_handle = p_priv->cs8k_handle;

    //pinmux_configure();

    switch (cmd)
    {
    case NIM_IOCTRL_CHANNEL_CHECK_LOCK:
        mt_fe_dmd_cs8k_sat_get_lock_state(cs8k_handle, &status, 0);
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
        port_m88cs8k_sat_set_lnb_voltage(handle, voltage, p_priv->cfg.pin_config.vsel_when_13v);
        p_priv->lnb_polar = (MT_U8)param;
        p_priv->lnb_voltage = voltage;
        break;
    case NIM_IOCTRL_SET_LNB_ONOFF:
        //MT_INFO_FRONTEND("ker cs8k_sat line:%d param=0x%08x\n", __LINE__, param);
        if (p_priv->lnb_onoff == param)
        {
            break;
        }

        //MT_INFO_FRONTEND("NIM_IOCTRL_SET_LNB_ONOFF set %d\n", param);
        port_m88cs8k_sat_set_lnb_onoff(handle, (MT_U8)param, &p_priv->cfg.pin_config);
        p_priv->lnb_onoff = param;
        if (param == 0)
        {
            break;
        }

        /* restore voltage */
        port_m88cs8k_sat_set_lnb_voltage(handle, p_priv->lnb_voltage, p_priv->cfg.pin_config.vsel_when_13v);
        /* restore 22k */
        if (p_priv->onoff_22k != 0)
        {
            port_m88cs8k_sat_set_22k_onoff(handle, p_priv->onoff_22k, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);
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
            //port_m88cs8k_sat_diseqc_ctrl(handle, &diseqc_cmd);
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
          //port_dm6k_lnb_sc_restore(handle, &p_priv->cfg.pin_config);
        }
        break;

    case NIM_IOCTRL_REMOVE_PROTECT:
        if (1 == p_priv->cfg.lnb_prot_by_mcu)
        {
          //port_dm6k_lnb_sc_remove(&p_priv->cfg.pin_config);
        }
        break;

    case NIM_IOCTRL_ENABLE_CHECK_PROTECT:
        if (1 == p_priv->cfg.lnb_prot_by_mcu)
        {
          //port_dm6k_lnb_sc_chk_enable(&p_priv->cfg.pin_config);
        }
        break;
#endif

    case NIM_IOCTRL_SET_22K_ONOFF:
        //MT_INFO_FRONTEND("ker cs8k_sat ioctl line:%d param=0x%08x\n", __LINE__, param);
        if (p_priv->onoff_22k == param)
        {
            break;
        }
        port_m88cs8k_sat_set_22k_onoff(handle, (MT_U8)param, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);
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
        //port_dm6k_recover(handle);
        break;

    case NIM_IOCTRL_SET_CHANNEL_INFO:
        //rc = port_dm6k_set_chninfo(handle, param);
        return rc;
        break;
#endif

    case NIM_IOCTRL_GET_SIGNAL_INFO:
        port_m88cs8k_sat_get_signal_info(handle, (mt_unf_fe_signal_info_t *)param);
        break;

#if 0
    case NIM_IOCTRL_GET_CHANNEL_INFO:
        if (param != 0)
          memcpy((MT_UNF_FE_CHANNEL_INFO_S*)param, &p_priv->cur_channel, sizeof(MT_UNF_FE_CHANNEL_INFO_S));
        break;
#endif

    case NIM_IOCTRL_SCAN_CANCEL:
        port_m88cs8k_sat_blind_scan_cancel(p_priv);
        break;

    case NIM_IOCTRL_GET_SCAN_STATUS:
        //mutex_lock(&bs_notify_status_lock);
        if (param != 0)
            *((MT_U8 *)param) = cs8k_sat_notify_scan_status;
        //mutex_unlock(&bs_notify_status_lock);
        break;

    case NIM_IOCTRL_GET_SCAN_RESULT:
        p_scan_info = (fe_blindscan_param_t *)param;
        if (p_priv->scan_info.channel_num_total)
        {
            if (param != 0)
                memcpy((fe_blindscan_param_t *)param, &p_priv->scan_info, sizeof(fe_blindscan_param_t));
        }
        break;

    case NIM_IOCTRL_DISEQC_SENDMSG:
        p_sendmsg = (mt_unf_fe_diseqc_sendmsg_t *)param;
        port_m88cs8k_sat_diseqc_sendmsg(handle, p_sendmsg);
        break;

    case NIM_IOCTRL_DISEQC_SEND_TONEBURST:
        //p_diseqc_cmd = (mt_unf_fe_diseqc_sendmsg_t*)param;
        port_m88cs8k_sat_diseqc_send_tone_burst(handle, param);
        break;
    case NIM_IOCTRL_DISEQC_RECVMSG:
        p_recvmsg = (mt_unf_fe_diseqc_recvmsg_t *)param;
        port_m88cs8k_sat_diseqc_recvmsg(handle, p_recvmsg);
        break;
    //#endif
    case NIM_IOCTRL_SAT_BS_EVENT_PROCESSED:
#ifdef CFG_SYNC_BLINDSCAN
    {
        //printk("%s() %d: NIM_IOCTRL_SAT_BS_EVENT_PROCESSED\n", __FUNCTION__, __LINE__);
        event_status = 1;
        wake_up_interruptible(&bs_cb_wq);
    }
#endif
    break;
    default:
        break;
    }

    return MT_SUCCESS;
}

static int port_m88cs8k_sat_blind_scan_cancel(void *handle)
{
    mt_fe_cs8k_sat_priv_handle p_priv = (mt_fe_cs8k_sat_priv_handle)handle;
    MT_FE_CS8000_SAT_Device_Handle cs8k_handle = p_priv->cs8k_handle;

    MT_INFO_FRONTEND("port_m88cs8k_sat_blind_scan_cancel\n");
    mt_fe_dmd_cs8k_sat_blindscan_abort(cs8k_handle, MtFe_True);

    return MT_SUCCESS;
}

static const struct nla_policy cs8k_sat_policy[CS8K_SAT_NL_FAMILY_ATTR_AMOUNT] = {
    [CS8K_SAT_NL_ATTR_DATA0] = {.type = NLA_BINARY, .len = sizeof(struct cs8k_sat_netlink_data)},
};

static int cs8k_sat_nl_recvmsg0(struct sk_buff *skb, struct genl_info *info)
{
    struct cs8k_sat_netlink_data *cs8k_sat_nl_data;
    struct cs8k_sat_netlink_pid_list *cs8k_sat_nl_pid;
    int err = 0;

    //MT_INFO_FRONTEND("%s\n", __FUNCTION__);

    if (info->attrs[CS8K_SAT_NL_ATTR_DATA0])
    {
        cs8k_sat_nl_data = nla_data(info->attrs[CS8K_SAT_NL_ATTR_DATA0]);
        cs8k_sat_nl_pid = kzalloc(sizeof(struct cs8k_sat_netlink_pid_list), GFP_KERNEL);
        if (cs8k_sat_nl_pid == NULL)
            return MtFeErr_NoMemory;
        cs8k_sat_nl_pid->net = sock_net(skb->sk);
        /* copy from sk_buff, because we will use it in other thread, and sk_buff will be released soon */
        cs8k_sat_nl_pid->cs8k_sat_nl_data = *cs8k_sat_nl_data;
        //MT_INFO_FRONTEND("netlink : app_pid = %d\n", NETLINK_CB(skb).portid);
        //MT_INFO_FRONTEND("netlink : pid = %d\n", cs8k_sat_nl_data->pid);
        //MT_INFO_FRONTEND("netlink : seq = 0x%x\n", cs8k_sat_nl_data->seq);
        //MT_INFO_FRONTEND("netlink : cmd = %d\n", cs8k_sat_nl_data->cmd);
        //MT_INFO_FRONTEND("netlink : data01 = %d\n", cs8k_sat_nl_data->data01);
        //MT_INFO_FRONTEND("netlink : name : %s, freq = %u, sym = %u\n", cs8k_sat_nl_data->name, cs8k_sat_nl_data->freq_khz, cs8k_sat_nl_data->symbol_rate);

        if (mutex_lock_interruptible(&cs8k_sat_nl_mutex))
        {
            kfree(cs8k_sat_nl_pid);
            return -ETIME;
        }
        list_add_tail(&(cs8k_sat_nl_pid->list), &cs8k_sat_nl_pid_list);
        mutex_unlock(&cs8k_sat_nl_mutex);
    }

    return err;
}

static struct genl_ops cs8k_sat_nl_ops[CS8K_SAT_NL_OPS_AMOUNT] = {
    [CS8K_SAT_NL_OPS_CMD0] = {
        .cmd = CS8K_SAT_NL_OPS_CMD0,
        .doit = cs8k_sat_nl_recvmsg0,
        .policy = cs8k_sat_policy,
    },
};

static int port_m88cs8k_sat_blind_scan_start(void *pArg)
{
    S32 ret = 0;
    U32 i = 0;
    MT_FE_CS8000_SAT_Device_Handle cs8k_handle = NULL;
    fe_blindscan_param_t *p_scan_info = (fe_blindscan_param_t *)pArg;

    cs8k_handle = (MT_FE_CS8000_SAT_Device_Handle)g_cs8k_sat_priv->cs8k_handle;
    MT_INFO_FRONTEND("ker cs8k_sat: start-stop: start feq:%d end_freq: %d\n",
                     p_scan_info->start_freq, p_scan_info->stop_freq);
    MT_INFO_FRONTEND("ker cs8k_sat: use_uc[%d] bank[%d] user_band[%d] ub_freq_mhz[%d]\n",
                     p_scan_info->uc_param.use_uc,
                     p_scan_info->uc_param.bank,
                     p_scan_info->uc_param.user_band,
                     p_scan_info->uc_param.ub_freq_mhz);

    cs8k_handle->lnb_cfg.bUnicable = p_scan_info->uc_param.use_uc;
    cs8k_handle->lnb_cfg.iBankIndex = p_scan_info->uc_param.bank;
    cs8k_handle->lnb_cfg.iUBIndex = p_scan_info->uc_param.user_band;
    cs8k_handle->lnb_cfg.iUBFreqMHz = p_scan_info->uc_param.ub_freq_mhz;

    g_cs8k_sat_priv->bs_info.tp_num = 0;

    //nim_lock(priv->drv_base);

#ifdef OLD_BLINDSCAN_ALGO
    ret = mt_fe_dmd_cs8k_sat_blindscan(cs8k_handle,
                                       p_scan_info->start_freq / 1000,
                                       p_scan_info->stop_freq / 1000,
                                       &g_cs8k_sat_priv->bs_info,
                                       1);
#else
    ret = mt_fe_dmd_cs8k_sat_blindscan_test(cs8k_handle,
                                            p_scan_info->start_freq / 1000,
                                            p_scan_info->stop_freq / 1000,
                                            &g_cs8k_sat_priv->bs_info,
                                            1);
#endif
    //nim_unlock(priv->drv_base);

    //MT_INFO_FRONTEND("nim_cs8k_service end. tp start %d end %d ret = %d\n",
    //g_cs8k_sat_priv->scan_info.start_freq, g_cs8k_sat_priv->scan_info.end_freq, ret);

    if (ret == MtFeErr_Ok)
    {
        p_scan_info = &g_cs8k_sat_priv->scan_info;
        p_scan_info->channel_num_cur = g_cs8k_sat_priv->bs_info.tp_num;

        /* Bug 108019 */
        p_scan_info->channel_num_total = 0;

        //MT_INFO_FRONTEND("port_m88cs8k_sat_blind_scan get %u tp:\n", g_cs8k_sat_priv->bs_info.tp_num);
        for (i = 0; i < g_cs8k_sat_priv->bs_info.tp_num; i++)
        {
            p_scan_info->p_channel_info[p_scan_info->channel_num_total].frequency =
                g_cs8k_sat_priv->bs_info.p_tp_info[i].freq_KHz;
            p_scan_info->p_channel_info[p_scan_info->channel_num_total].symbol_rate =
                g_cs8k_sat_priv->bs_info.p_tp_info[i].sym_rate_KSs;
            p_scan_info->p_channel_info[p_scan_info->channel_num_total].port_type =
                port_m88cs8k_sat_deparse_dvb_type(g_cs8k_sat_priv->bs_info.p_tp_info[i].dvb_type);
            p_scan_info->p_channel_info[p_scan_info->channel_num_total].fec_inner =
                port_m88cs8k_sat_deparse_code_rate(g_cs8k_sat_priv->bs_info.p_tp_info[i].code_rate);
            //MT_INFO_FRONTEND("feq:%u, symrate:%u, type:%u\n",
            //p_scan_info->p_channel_info[p_scan_info->channel_num_total].frequency,
            //p_scan_info->p_channel_info[p_scan_info->channel_num_total].symbol_rate,
            //p_scan_info->p_channel_info[p_scan_info->channel_num_total].port_type);
            p_scan_info->channel_num_total += 1;
        }

        //if you scan by dividing all channel to several "channel group", and then scan group by group
        //you should use p_scan_info->channel_num_cur to count scanned tp num for every scan.
        MT_INFO_FRONTEND("total channel_num_cur:%u channel_num_total:%d\n", p_scan_info->channel_num_cur, p_scan_info->channel_num_total);

        return MT_SUCCESS;
    }

    return MT_FAILURE;
}

static int port_m88cs8k_sat_blind_scan(void *handle, fe_blindscan_param_t *p_scan_info)
{
    //mutex_lock(&bs_notify_status_lock);
    cs8k_sat_notify_scan_status = 0;
    //mutex_unlock(&bs_notify_status_lock);

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 42)
    kernel_thread(port_m88cs8k_sat_blind_scan_start, p_scan_info, CLONE_KERNEL);
#else
    kthread_run(port_m88cs8k_sat_blind_scan_start, (void *)p_scan_info, "port_m88cs8k_sat_blind_scan");
#endif

    return MT_SUCCESS;
}

void port_m88cs8k_sat_notify_to_up_layer(MT_FE_MSG msg, void *p_param)
{
    //U16 i = 0;
    mt_unf_fe_blind_scan_channel_info_t *p_bs_channel_info = NULL;

    if (p_param != NULL)
        p_bs_channel_info = (mt_unf_fe_blind_scan_channel_info_t *)p_param;

    if (msg == MtFeMsg_BSFinish || msg == MtFeMsg_BSAbort)
    {
        cs8k_sat_notify_scan_status = 1;
        MT_INFO_FRONTEND("cs8k_sat_notify_scan_status = %d\n", cs8k_sat_notify_scan_status);
    }
}

static void port_m88cs8k_sat_register_to_drv_notify(MT_FE_CS8000_SAT_Device_Handle cs8k_handle, MT_FE_MSG msg, void *p_param)
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

    //MT_FE_CS8000_SAT_Device_Handle cs8k_handle = g_cs8k_sat_priv->cs8k_handle;

    /*
    if (port_m88cs8k_sat_notify_to_up_layer == NULL)
    {
        return;
    }
    */

    MT_INFO_FRONTEND("cs8k_notify_function: msg ");

    switch (msg)
    {
    case MtFeMsg_BSStart:
        MT_INFO_FRONTEND("BSTpStart\n");
        break;
    case MtFeMsg_BSOneWinFinish:
        MT_INFO_FRONTEND("BSOneWinFinish: feq %d\n", *(u32 *)p_param);
        break;
    case MtFeMsg_BSTpFind:
#if 0//def CFG_SYNC_BLINDSCAN
        init_waitqueue_head(&bs_cb_wq);

        event_status = 0;
#endif
        printk("BSTpFind: feq[%d] sym[%d]\n", p_tp_info[0].freq_KHz, p_tp_info[0].sym_rate_KSs);
        channel_info.frequency = p_tp_info[0].freq_KHz;
        channel_info.symbol_rate = p_tp_info[0].sym_rate_KSs;
        channel_info.port_type = MT_UNF_PORT_TYPE_DVBS_AUTO;
        cs8k_sat_nl_sendmsg(&channel_info);

#if 0//def CFG_SYNC_BLINDSCAN
        //printk("%s() %d: prepare to wait event, wq = 0x%08x, evt_status = %d\n", __FUNCTION__, __LINE__, &bs_cb_wq, event_status);

        error = wait_event_interruptible_timeout(bs_cb_wq, (event_status == 1), BLINDSCAN_WAIT_CB_TIMEOUT * HZ);
        if (error == 0)
        {
            printk("%s: blindscan wait callback timeout!\n", __FUNCTION__);
        }
        else if (error == -ERESTARTSYS)
        {
            printk("%s: blindscan interrupted by a signal!\n", __FUNCTION__);
        }
        printk("%s() %d: wait event OK or timeout, evt_status = %d\n", __FUNCTION__, __LINE__, event_status);
#endif
        break;
    case MtFeMsg_BSTpUnlock:
#if 0//def CFG_SYNC_BLINDSCAN
        init_waitqueue_head(&bs_cb_wq);

        event_status = 0;
#endif
        printk("BSTpUnlock: feq[%d] sym[%d]\n", p_tp_info[0].freq_KHz, p_tp_info[0].sym_rate_KSs);
        bs_channel_info.frequency = p_tp_info[0].freq_KHz;
        bs_channel_info.symbol_rate = p_tp_info[0].sym_rate_KSs;
        bs_channel_info.lock = 0;
#if 0
	mt_fe_dmd_cs8k_sat_get_strength(cs8k_handle, &s_strength);
	bs_channel_info.perf.agc =  s_strength;
	mt_fe_dmd_cs8k_sat_get_sat_quality(cs8k_handle, &snr);
	bs_channel_info.perf.snr = snr;
	mt_fe_dmd_cs8k_sat_get_per(cs8k_handle, &total_packages, &err_packages);
	bs_channel_info.perf.ber = (double)err_packages/(double)total_packages;
#endif
        port_m88cs8k_sat_notify_to_up_layer(MtFeMsg_BSTpUnlock, &bs_channel_info);

        channel_info.frequency = p_tp_info[0].freq_KHz;
        channel_info.symbol_rate = p_tp_info[0].sym_rate_KSs;
        channel_info.port_type = MT_UNF_PORT_TYPE_DVBS_BUTT;
        cs8k_sat_nl_sendmsg(&channel_info);

#if 0//def CFG_SYNC_BLINDSCAN
        //printk("%s() %d: prepare to wait event, wq = 0x%08x, evt_status = %d\n", __FUNCTION__, __LINE__, &bs_cb_wq, event_status);

        error = wait_event_interruptible_timeout(bs_cb_wq, (event_status == 1), BLINDSCAN_WAIT_CB_TIMEOUT * HZ);
        if (error == 0)
        {
            printk("%s: blindscan wait callback timeout!\n", __FUNCTION__);
        }
        else if (error == -ERESTARTSYS)
        {
            printk("%s: blindscan interrupted by a signal!\n", __FUNCTION__);
        }
        printk("%s() %d: wait event OK or timeout, evt_status = %d\n", __FUNCTION__, __LINE__, event_status);
#endif
        break;
    case MtFeMsg_BSTpLocked:
        bs_channel_info.frequency = p_tp_info[0].freq_KHz;
        bs_channel_info.symbol_rate = p_tp_info[0].sym_rate_KSs;
        bs_channel_info.lock = 1;
        /*
	bs_channel_info.nim_type = port_m88cs8k_sat_deparse_dvb_type(p_tp_info[0].dvb_type);
	bs_channel_info.param.dvbs.fec_inner = port_m88cs8k_sat_deparse_code_rate(p_tp_info[0].code_rate);
	mt_fe_dmd_cs8k_sat_get_strength(cs8k_handle, &s_strength);
	bs_channel_info.param.dvbs.perf.agc =  s_strength;
	mt_fe_dmd_cs8k_sat_get_sat_quality(cs8k_handle, &snr);
	bs_channel_info.param.dvbs.perf.snr = snr;
	mt_fe_dmd_cs8k_sat_get_per(cs8k_handle, &total_packages, &err_packages);
	bs_channel_info.param.dvbs.perf.ber = (double)err_packages/(double)total_packages;
	*/
        port_m88cs8k_sat_notify_to_up_layer(MtFeMsg_BSTpLocked, &bs_channel_info);
        printk("BSTpLocked: feq[%d] sym[%d]\n", p_tp_info[0].freq_KHz, p_tp_info[0].sym_rate_KSs);

#ifdef CFG_SYNC_BLINDSCAN
        init_waitqueue_head(&bs_cb_wq);

        event_status = 0;
#endif

        //kobject_uevent();
        channel_info.frequency = p_tp_info[0].freq_KHz;
        channel_info.symbol_rate = p_tp_info[0].sym_rate_KSs;
        channel_info.port_type = port_m88cs8k_sat_deparse_dvb_type(p_tp_info[0].dvb_type);

#ifdef CONFIG_NET
        cs8k_sat_nl_sendmsg(&channel_info);
#endif

#ifdef CFG_SYNC_BLINDSCAN
        //printk("%s() %d: prepare to wait event, wq = 0x%08x, evt_status = %d\n", __FUNCTION__, __LINE__, &bs_cb_wq, event_status);

        error = wait_event_interruptible_timeout(bs_cb_wq, (event_status == 1), BLINDSCAN_WAIT_CB_TIMEOUT * HZ);
        if (error == 0)
        {
            MT_ERR_FRONTEND("%s: blindscan wait callback timeout!\n", __FUNCTION__);
        }
        else if (error == -ERESTARTSYS)
        {
            MT_ERR_FRONTEND("%s: blindscan interrupted by a signal!\n", __FUNCTION__);
        }
        //printk("%s() %d: wait event OK or timeout, evt_status = %d\n", __FUNCTION__, __LINE__, event_status);
#endif

        break;
    case MtFeMsg_BSAbort:
        MT_INFO_FRONTEND("BSAbort \n");
        cs8k_sat_nl_list_del();
        port_m88cs8k_sat_notify_to_up_layer(MtFeMsg_BSAbort, NULL);
        break;
    case MtFeMsg_BSFinish:
        MT_INFO_FRONTEND("BSTpFinish \n");
        cs8k_sat_nl_list_del();
        port_m88cs8k_sat_notify_to_up_layer(MtFeMsg_BSFinish, NULL);
        break;
    default:
        break;
    }
}

static void port_m88cs8k_sat_set_rfagc_mode(void)
{
    mt_u32 reg_base = 0;
    mt_u32 temp;

    reg_base = mt_get_public_base();
    /*dvbs/dvbs2 rf-agc use cmos mode*/
    temp = readl((volatile int *)(reg_base + 0xc018));
    temp |= 0x01;
    writel(temp, (volatile int *)(reg_base + 0xc018));
}

int m88cs8k_sat_attach(frontend_info_s *info, mt_unf_fe_attr_t *attr)
{
    mt_fe_cs8k_sat_priv_handle p_priv = NULL;
    MT_FE_CS8000_SAT_Device_Handle dev_handle = NULL;
    mt_u8 reg_val = 0;
    int ret = 0;

    info->ops.connect = port_m88cs8k_sat_channel_connect;
    info->ops.get_status = port_m88cs8k_sat_get_status;
    info->ops.get_ber = port_m88cs8k_sat_get_ber;
    info->ops.get_snr = port_m88cs8k_sat_get_snr;
    info->ops.get_signal_strength = port_m88cs8k_sat_get_signal_strength;
    info->ops.get_signal_quality = port_m88cs8k_sat_get_signal_quality;
    info->ops.get_signal_agc = port_m88cs8k_sat_get_signal_agc;

    info->ops.standby = port_m88cs8k_sat_standby;
    info->ops.wakeup = port_m88cs8k_sat_wakeup;
    info->ops.get_default_timeout = port_m88cs8k_sat_get_default_timeout;
    info->ops.set_io = port_m88cs8k_sat_set_io;
    info->ops.port_ioctl = port_m88cs8k_sat_ioctl;
    info->ops.blind_scan = port_m88cs8k_sat_blind_scan;

    p_priv = kzalloc(sizeof(mt_fe_cs8k_sat_priv_t), GFP_KERNEL);
    if (p_priv == NULL)
        return -ENOMEM;

    dev_handle = kzalloc(sizeof(MT_FE_CS8000_SAT_DEVICE_SETTINGS), GFP_KERNEL);
    if (dev_handle == NULL)
    {
        kfree((void *)p_priv);
        g_cs8k_sat_priv = NULL;
        return -ENOMEM;
    }
    p_priv->cs8k_handle = dev_handle;
    info->handle = (void *)p_priv;
    g_cs8k_sat_priv = p_priv;

    g_i2c_cs8k_sat = attr->demod_i2c_id;
    dev_handle->demod_dev_addr = attr->demod_addr;           //0xd0;
    dev_handle->tuner_cfg.tuner_dev_addr = attr->tuner_addr; //0xc0;
    //MT_INFO_FRONTEND("line[%d] i2c%d demod_addr=0x%02x tuner_addr=0x%02x\n", __LINE__, g_i2c_cs8k_sat, attr->u32DemodAddr, attr->u32TunerAddr);

    memcpy(&(p_priv->cfg), &(attr->fe_config), sizeof(mt_unf_fe_config_para_t));

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

    if (NULL == cs8k_sat_pg_channel_info)
    {
        cs8k_sat_pg_channel_info = kzalloc(sizeof(mt_unf_fe_channel_info_t) * MAX_BS_TP_NUM_PER_SAT, GFP_KERNEL);
        if (cs8k_sat_pg_channel_info == NULL)
        {
            kfree(dev_handle);
            kfree((void *)p_priv);
            g_cs8k_sat_priv = NULL;
            return -ENOMEM;
        }
    }
    p_priv->scan_info.p_channel_info = cs8k_sat_pg_channel_info;

    p_priv->bs_info.bs_times = p_priv->cfg.bs_times;
    p_priv->bs_info.tp_max_num = MAX_TP_ONE_SCAN;
    if (NULL == p_priv->bs_info.p_tp_info)
    {
        p_priv->bs_info.p_tp_info = kzalloc(sizeof(MT_FE_TP_INFO) * p_priv->bs_info.tp_max_num, GFP_KERNEL);
        if (p_priv->bs_info.p_tp_info == NULL)
        {
            kfree(dev_handle);
            kfree((void *)p_priv);
            if (cs8k_sat_pg_channel_info)
            {
                kfree(cs8k_sat_pg_channel_info);
                cs8k_sat_pg_channel_info = NULL;
            }
            g_cs8k_sat_priv = NULL;
            return -ENOMEM;
        }
    }
    p_priv->bs_info.bs_algorithm = MT_FE_BS_ALGORITHM_A;
    port_m88cs8k_sat_set_rfagc_mode();
    mt_fe_dmd_cs8k_sat_config_default(dev_handle);

    if (attr->tuner_type == MT_UNF_TUNER_TYPE_M88TS2022)
        mt_fe_dmd_cs8k_sat_select_tuner(dev_handle, TN_MONTAGE_TS2022);
    else if (attr->tuner_type == MT_UNF_TUNER_TYPE_M88TS6011)
        mt_fe_dmd_cs8k_sat_select_tuner(dev_handle, TN_MONTAGE_TS6011);

    //p_priv->cs8k_handle->tuner_cfg.tuner_clock_out = p_priv->cfg.tun_clk_out_cfg;

    ret = mt_fe_dmd_cs8k_sat_init(dev_handle);
    if (ret < 0)
    {
        kfree(dev_handle);
        kfree((void *)p_priv);
        if (cs8k_sat_pg_channel_info)
        {
            kfree(cs8k_sat_pg_channel_info);
            cs8k_sat_pg_channel_info = NULL;
        }

        if (p_priv->bs_info.p_tp_info)
        {
            kfree(p_priv->bs_info.p_tp_info);
            p_priv->bs_info.p_tp_info = NULL;
        }
        g_cs8k_sat_priv = NULL;
        return MT_FAILURE;
    }
    dev_handle->dmd_get_reg(dev_handle, 0x00, &reg_val);
    if (reg_val != 0xe0)
    {
        kfree(dev_handle);
        kfree((void *)p_priv);
        if (cs8k_sat_pg_channel_info)
        {
            kfree(cs8k_sat_pg_channel_info);
            cs8k_sat_pg_channel_info = NULL;
        }

        if (p_priv->bs_info.p_tp_info)
        {
            kfree(p_priv->bs_info.p_tp_info);
            p_priv->bs_info.p_tp_info = NULL;
        }
        g_cs8k_sat_priv = NULL;
        return MT_FAILURE;
    }

    memcpy(&info->pre_attr, attr, sizeof(mt_unf_fe_attr_t));

    mt_fe_dmd_cs8k_sat_register_notify(port_m88cs8k_sat_register_to_drv_notify);

    p_priv->lnbctrl_dev = attr->fe_config.lnbctrl_dev;
    p_priv->lnb_dev_addr = attr->fe_config.lnb_dev_addr;
    p_priv->lnb_i2c_id = attr->fe_config.lnb_i2c_id;

    /* default 22k off */
    port_m88cs8k_sat_set_22k_onoff(p_priv, 0, p_priv->cfg.pin_config.diseqc_out_when_lnb_off);
    /* default lnb off */
    port_m88cs8k_sat_set_lnb_onoff(p_priv, 0, &p_priv->cfg.pin_config);

#ifdef CONFIG_NET
    ret = genl_register_family(&cs8k_sat_nl_family);
    if (ret)
    {
        MT_INFO_FRONTEND("genl_register_family failed, ret=0x%08x\n", ret);
        kfree(dev_handle);
        kfree((void *)p_priv);
        if (cs8k_sat_pg_channel_info)
        {
            kfree(cs8k_sat_pg_channel_info);
            cs8k_sat_pg_channel_info = NULL;
        }

        if (p_priv->bs_info.p_tp_info)
        {
            kfree(p_priv->bs_info.p_tp_info);
            p_priv->bs_info.p_tp_info = NULL;
        }

        g_cs8k_sat_priv = NULL;
        return -EINVAL;
    }
    MT_INFO_FRONTEND("cs8k_sat_nl_family.id = 0x%x\n", cs8k_sat_nl_family.id);
#endif

    info->is_attach = 1;

    return MT_SUCCESS;
}

int m88cs8k_sat_detach(frontend_info_s *info)
{
    mt_fe_cs8k_sat_priv_handle p_priv = NULL;
    MT_FE_CS8000_SAT_Device_Handle dev_handle = NULL;

    if (info->is_attach)
    {
        p_priv = info->handle;
        dev_handle = p_priv->cs8k_handle;

        if (p_priv->bs_info.p_tp_info)
        {
            kfree(p_priv->bs_info.p_tp_info);
            p_priv->bs_info.p_tp_info = NULL;
        }

        kfree(dev_handle);
        kfree((void *)p_priv);
        if (cs8k_sat_pg_channel_info)
        {
            kfree(cs8k_sat_pg_channel_info);
            cs8k_sat_pg_channel_info = NULL;
        }

        info->is_attach = 0;
    }

    g_cs8k_sat_priv = NULL;

#ifdef CONFIG_NET
    genl_unregister_family(&cs8k_sat_nl_family);
#endif

    return MT_SUCCESS;
}
