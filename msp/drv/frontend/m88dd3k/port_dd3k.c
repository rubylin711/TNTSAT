/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
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

#include "mt_type.h" //flax

//#include <drv_frontend.h>
#include "mt_unf_frontend.h"

#include "mt_fe_common.h"
#include "port_dd3k.h"
#include "drv_frontend_ioctl.h"

#include "mt_fe_i2c.h"

#include "mt_module_debug.h"

//#define FOR_PORT_DD3K_CONNECT_SYCHRONOUS

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

/* The crypto netlink socket */
//static struct sock *dd3k_nlsk;

mt_fe_dd3k_priv_handle g_dd3k_priv = NULL;
int g_i2c_dd3k = 0;

static U16 g_cur_dvbc_qam = 64;

static int port_m88dd3k_channel_set(void *handle, mt_unf_fe_connect_para_t *para)
{
	MT_FE_RET ret = 0;
	MT_FE_TYPE dvb_type = MtFeType_Undef;
	mt_unf_fe_channel_info_t *p_channel_info = NULL;
	mt_fe_dd3k_priv_handle p_priv = (mt_fe_dd3k_priv_handle)handle;
	MT_FE_DD_Device_Handle dd3k_handle = p_priv->dd3k_handle;

	U32 freq, sym_rate, xtal = 28800;
	U16 qam, inverted;

	p_channel_info = &(para->channel_info);
	para->channel_info.lock = 0;

	if (para->sig_type == MT_UNF_FE_SIG_TYPE_CAB)
	{
		dvb_type = MtFeType_DVBC;

		freq     = para->connect_param.cab.freq * 1000;
		sym_rate = para->connect_param.cab.sym_rate / 1000;
		inverted = para->connect_param.cab.b_reverse;

		switch (para->connect_param.cab.mod_type)
		{
			case MT_UNF_MOD_TYPE_QAM_16:
				qam = 16;
				break;

			case MT_UNF_MOD_TYPE_QAM_32:
				qam = 32;
				break;

			case MT_UNF_MOD_TYPE_QAM_128:
				qam = 128;
				break;

			case MT_UNF_MOD_TYPE_QAM_256:
				qam = 256;
				break;

			case MT_UNF_MOD_TYPE_QAM_64:
			default:
				qam = 64;
				break;
		}

		g_cur_dvbc_qam = qam;

		printk("%s[%d] DVBC: freq[%6d], sym[%4d], qam[%3d], inverted[%d], type[%d]\n", __FUNCTION__, __LINE__,
			   freq / 1000, sym_rate, qam, inverted, para->sig_type);

		//dd3k_handle->demod_cur_mode = dvb_type;

		mt_fe_change_mode_dd3k(dd3k_handle, dvb_type);

		memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));

		ret = mt_fe_dmd_connect_dd3k(dd3k_handle, freq, sym_rate, qam, inverted, xtal);

		para->channel_set_info.lock_time = 800;
	}
	else if (para->sig_type == MT_UNF_FE_SIG_TYPE_DTMB)
	{
		dvb_type = MtFeType_CTTB;

		freq = para->connect_param.ter.freq * 1000;

		printk("%s[%d] DTMB: freq[%6d], type[%d]\n", __FUNCTION__, __LINE__, freq / 1000, para->sig_type);

		memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));

		ret = mt_fe_dmd_connect_dd3k(dd3k_handle, freq, 0, 0, 0, 0);

		para->channel_set_info.lock_time = 2000;
	}

	if (ret == MtFeErr_Ok)
	{
		return MT_SUCCESS;
	}

	return MT_FAILURE;
}

static int port_m88dd3k_get_status(void *handle, mt_unf_fe_status_t *p_status)
{
	mt_fe_dd3k_priv_handle p_priv = (mt_fe_dd3k_priv_handle)handle;
	MT_FE_DD_Device_Handle dd3k_handle = p_priv->dd3k_handle;
	MT_FE_LOCK_STATE stat = 0;

	mt_fe_dmd_get_lock_state_dd3k(dd3k_handle, &stat);

	if (stat == MtFeLockState_Locked)
	{
		p_status->lock_status = MT_UNF_FE_SIGNAL_LOCKED;

		if (dd3k_handle->demod_cur_mode == MtFeType_CTTB)
		{
			S8 _snr = 0;
			S8 _strength = 0;

			dd3k_handle->tuner_ct_settings.tuner_get_strength(dd3k_handle, &_strength);
			mt_fe_dmd_get_quality_dd3k_t(dd3k_handle, &_snr);

			printk("%s[%d] ---- DTMB Locked! Signal strength = %d, SNR = %d\n", __FUNCTION__, __LINE__, _strength, _snr);
		}
	}
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

static int port_m88dd3k_get_signal_quality(void *handle, MT_U32 *p_quality)
{
	mt_fe_dd3k_priv_handle p_priv = (mt_fe_dd3k_priv_handle)handle;
	MT_FE_DD_Device_Handle dd3k_handle = p_priv->dd3k_handle;
	int ret = 0;
	U8 percent = 0;
	S8 snr = 0;

	U8 qam = 64;

	if (dd3k_handle->demod_cur_mode == MtFeType_DVBC)
	{
		ret = _mt_fe_dmd_get_snr_dd3k_c(dd3k_handle, qam, &percent);
		*p_quality = percent;
	}
	else
	{
		ret = mt_fe_dmd_get_quality_dd3k_t(dd3k_handle, &snr);
		*p_quality = snr;
	}

	if (ret < 0)
	{
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

static int port_m88dd3k_get_ber(void *handle, MT_U32 *p_ber)
{
	mt_fe_dd3k_priv_handle p_priv = (mt_fe_dd3k_priv_handle)handle;
	MT_FE_DD_Device_Handle dd3k_handle = p_priv->dd3k_handle;
	U32 error_packages = 0;
	U32 total_packages = 0;

	if (dd3k_handle->demod_cur_mode == MtFeType_DVBC)
	{		// DVB-C
		_mt_fe_dmd_get_ber_dd3k_c(dd3k_handle, &error_packages, &total_packages);
	}
	else	// DTMB
	{
		U16 error_cnt = 0, total_cnt = 0;

		mt_fe_dmd_get_per_dd3k_t(dd3k_handle, &error_cnt, &total_cnt);

		error_packages = error_cnt;
		total_packages = total_cnt;
	}

	p_ber[0] = total_packages;
	p_ber[1] = error_packages;
	p_ber[2] = 0;

	return MT_SUCCESS;
}

static int port_m88dd3k_get_snr(void *handle, MT_U32 *p_snr)
{
	mt_fe_dd3k_priv_handle p_priv = (mt_fe_dd3k_priv_handle)handle;
	MT_FE_DD_Device_Handle dd3k_handle = p_priv->dd3k_handle;
	S8 _snr = 0;

	if (dd3k_handle->demod_cur_mode == MtFeType_DVBC)
	{		// DVB-C
		U8 dvbc_snr = 0;

		_mt_fe_dmd_get_snr_dd3k_c(dd3k_handle, g_cur_dvbc_qam, &dvbc_snr);

		_snr = (S8)dvbc_snr;
	}
	else	// DTMB
	{
		mt_fe_dmd_get_quality_dd3k_t(dd3k_handle, &_snr);
	}

	*p_snr = _snr;

	return MT_SUCCESS;
}

static int port_m88dd3k_get_signal_strength(void *handle, MT_U32 *p_strength)
{
	mt_fe_dd3k_priv_handle p_priv = (mt_fe_dd3k_priv_handle)handle;
	MT_FE_DD_Device_Handle dd3k_handle = p_priv->dd3k_handle;
	S8 _strength = 0;

	if (dd3k_handle->demod_cur_mode == MtFeType_DVBC)
	{		// DVB-C
		dd3k_handle->tuner_dc_settings.tuner_get_strength(dd3k_handle, &_strength);
	}
	else	// DTMB
	{
		dd3k_handle->tuner_ct_settings.tuner_get_strength(dd3k_handle, &_strength);
	}

	*p_strength = _strength;

	return MT_SUCCESS;
}

static void port_m88dd3k_get_signal_info(void *handle, mt_unf_fe_signal_info_t *p_sig_info)
{
	mt_fe_dd3k_priv_handle p_priv = (mt_fe_dd3k_priv_handle)handle;
	MT_FE_DD_Device_Handle dd3k_handle = p_priv->dd3k_handle;

	if (p_sig_info->sig_type == MT_UNF_FE_SIG_TYPE_DTMB)
	{
		if (dd3k_handle->demod_cur_mode == MtFeType_CTTB)
			p_sig_info->sig_info.ter.ter_type = MT_UNF_FE_DTMB;
		else
			p_sig_info->sig_info.ter.ter_type = MT_UNF_FE_BUTT;
	}
	else
	{
		if (dd3k_handle->demod_cur_mode == MtFeType_DVBC)
			p_sig_info->sig_info.cab.cab_type = MT_UNF_FE_DVBC;
		else
			p_sig_info->sig_info.cab.cab_type = MT_UNF_FE_BUTT;
	}
}

static int port_m88dd3k_get_signal_agc(void *handle, mt_u32 *p_agc)
{
	//mt_fe_dd3k_priv_handle p_priv = (mt_fe_dd3k_priv_handle)handle;
	//MT_FE_DD_Device_Handle dd3k_handle = p_priv->dd3k_handle;
	//int ret = 0;
	//mt_u32 tuner_gain = 0;

	return MT_SUCCESS;
}

static int port_m88dd3k_standby(void *handle)
{
	mt_fe_dd3k_priv_handle p_priv = (mt_fe_dd3k_priv_handle)handle;
	MT_FE_DD_Device_Handle dd3k_handle = p_priv->dd3k_handle;

	mt_fe_dmd_sleep_dd3k(dd3k_handle);

	return MT_SUCCESS;
}

static int port_m88dd3k_wakeup(void *handle)
{
	mt_fe_dd3k_priv_handle p_priv = (mt_fe_dd3k_priv_handle)handle;
	MT_FE_DD_Device_Handle dd3k_handle = p_priv->dd3k_handle;

	mt_fe_dmd_wake_dd3k(dd3k_handle);

	return MT_SUCCESS;
}

static int port_m88dd3k_get_default_timeout(void *handle, mt_u32 *timeout)
{
	*timeout = 120;
	return MT_SUCCESS;
}

static int port_m88dd3k_set_io(void *handle, MT_BOOL onoff)
{
	return MT_SUCCESS;
}

static int port_m88dd3k_channel_connect(void *handle, mt_unf_fe_connect_para_t *para)
{
	mt_unf_fe_channel_info_t *p_channel_info = NULL;
	mt_fe_dd3k_priv_handle p_priv = (mt_fe_dd3k_priv_handle)handle;
	MT_FE_DD_Device_Handle dd3k_handle = p_priv->dd3k_handle;

	//MT_FE_RET ret = MtFeErr_Ok;
	MT_FE_LOCK_STATE status = 0;
	MT_U32 cnt = 0;

	//U8 tmp;
	MT_FE_TYPE dvb_type = MtFeType_Undef;

	p_priv->sig_type = para->sig_type;
	p_channel_info = &(para->channel_info);
	//pinmux_configure();

	//printk("----port_m88dd3k_channel_connect() log1, sig_type = %d\n", para->sig_type);

	if (para->sig_type == MT_UNF_FE_SIG_TYPE_DTMB)
	{
		dvb_type = MtFeType_CTTB;

		memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));
		para->channel_info.lock = 0;

		port_m88dd3k_channel_set(p_priv, para);

		//printk("lock_timeout_ms :%d\n", para->stChannelSetInfo.lock_time);
		for (cnt = 0; cnt < para->channel_set_info.lock_time; cnt += 10)
		{
			mt_fe_dmd_get_lock_state_dd3k(dd3k_handle, &status);
			para->channel_info.lock = (status == MtFeLockState_Locked) ? 1 : 0;
			if (para->channel_info.lock)
			{
				//printk("dd3k connect lock success\n");
				return MT_SUCCESS;
			}
			//mdelay(10);
			msleep(10);
		}

		return ERR_TIMEOUT;
	}
	else if (para->sig_type == MT_UNF_FE_SIG_TYPE_CAB)
	{
		dvb_type = MtFeType_DVBC;

#if 0
		printk("dd3k_channel set c: feq %d, symbol rate = %d, type = %d\n",
				para->connect_param.cab.freq,
				para->connect_param.cab.sym_rate,
				dvb_type);
#endif

		memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));
		para->channel_info.lock = 0;

		port_m88dd3k_channel_set(p_priv, para);

		//printk("lock_timeout_ms :%d\n", para->stChannelSetInfo.lock_time);
		for (cnt = 0; cnt < para->channel_set_info.lock_time; cnt += 10)
		{
			mt_fe_dmd_get_lock_state_dd3k(dd3k_handle, &status);
			para->channel_info.lock = (status == MtFeLockState_Locked) ? 1 : 0;
			if (para->channel_info.lock)
			{
				//printk("dd3k connect lock success\n");
				return MT_SUCCESS;
			}
			//mdelay(10);
			msleep(10);
		}

		return ERR_TIMEOUT;
	}

	return MT_FAILURE;
}

static int port_m88dd3k_ioctl(void *handle, mt_u32 cmd, mt_u32 param)
{
	MT_FE_LOCK_STATE status = 0;
	//U8 value = 0;
	//int rc = 0;

	mt_fe_dd3k_priv_handle p_priv = (mt_fe_dd3k_priv_handle)handle;
	MT_FE_DD_Device_Handle dd3k_handle = p_priv->dd3k_handle;
	//pinmux_configure();

	switch (cmd)
	{
		case NIM_IOCTRL_CHANNEL_CHECK_LOCK:
			mt_fe_dmd_get_lock_state_dd3k(dd3k_handle, &status);
			((mt_unf_fe_channel_info_t *)param)->lock = (status == MtFeLockState_Locked) ? 1 : 0;
			break;

		case NIM_IOCTRL_GET_TN_VERSION:
			//*((MT_U8 *)param) = p_priv->cfg.tun_support;
			break;

		case NIM_IOCTRL_GET_SIGNAL_INFO:
			port_m88dd3k_get_signal_info(handle, (mt_unf_fe_signal_info_t *)param);
			break;

		default:
			break;
	}

	return MT_SUCCESS;
}

#ifdef CONFIG_NET
int m88dd3k_attach(frontend_info_s *info, mt_unf_fe_attr_t *attr)
{
	mt_fe_dd3k_priv_handle p_priv = NULL;
	MT_FE_DD_Device_Handle dev_handle = NULL;
	//mt_u8 reg_val = 0;
	//int ret = 0;

	U32 data = 0;

	MT_FE_DD_DEMOD_SETTINGS		demod_ct_cfg;
	MT_FE_DD_DEMOD_SETTINGS		demod_dc_cfg;
	MT_FE_DD_TN_DEV_SETTINGS	tuner_ct_cfg;
	MT_FE_DD_TN_DEV_SETTINGS	tuner_dc_cfg;
	DVBC_QAM_CONFIG	dc_qam_cfg = {{64, 256, 128, 32, 16}, 5, 1600};


	info->ops.connect = port_m88dd3k_channel_connect;
	info->ops.get_status = port_m88dd3k_get_status;
	info->ops.get_ber = port_m88dd3k_get_ber;
	info->ops.get_snr = port_m88dd3k_get_snr;
	info->ops.get_signal_strength = port_m88dd3k_get_signal_strength;
	info->ops.get_signal_quality = port_m88dd3k_get_signal_quality;
	info->ops.get_signal_agc = port_m88dd3k_get_signal_agc;

	info->ops.standby = port_m88dd3k_standby;
	info->ops.wakeup = port_m88dd3k_wakeup;
	info->ops.get_default_timeout = port_m88dd3k_get_default_timeout;
	info->ops.set_io = port_m88dd3k_set_io;
	info->ops.port_ioctl = port_m88dd3k_ioctl;

	writel(0x00000100, (volatile void __iomem *)0xbf15b400);
	writel(0x00000000, (volatile void __iomem *)0xbf15b404);
	writel(0x11111111, (volatile void __iomem *)0xbf13c000);
	writel(0x00011001, (volatile void __iomem *)0xbf13c004);
	writel(0x11111110, (volatile void __iomem *)0xbf13c008);
	writel(0x10000111, (volatile void __iomem *)0xbf13c00c);
	writel(0x00001111, (volatile void __iomem *)0xbf13c010);

	data = readl((volatile void __iomem *)0xbf5d0094);
	data |= 0x01;
	writel(data, (volatile void __iomem *)0xbf5d0094);

	writel(0x000b0003, (volatile void __iomem *)0xbf500024);
	writel(0x00000310, (volatile void __iomem *)0xbf138008);

	writel(0x00000000, (volatile void __iomem *)0xbf200010);
	writel(0xc70b01ff, (volatile void __iomem *)0xbf200010);

	p_priv = kzalloc(sizeof(mt_fe_dd3k_priv_t), GFP_KERNEL);
	if (p_priv == NULL)
		return -ENOMEM;

	dev_handle = kzalloc(sizeof(MT_FE_DD_DEVICE_SETTINGS), GFP_KERNEL);
	if (dev_handle == NULL)
	{
		kfree((void *)p_priv);
		g_dd3k_priv = NULL;
		return -ENOMEM;
	}

	p_priv->dd3k_handle = dev_handle;
	info->handle = (void *)p_priv;
	g_dd3k_priv = p_priv;

	g_i2c_dd3k = attr->demod_i2c_id;

	memcpy(&(p_priv->cfg), &(attr->fe_config), sizeof(mt_unf_fe_config_para_t));

	mt_fe_demod_settings_default_dd3k(dev_handle);

	demod_ct_cfg.demod_mode = MtFeType_CTTB;
	demod_ct_cfg.demod_bandwidth = MtFeBandwidth_8M;
	demod_ct_cfg.demod_i2c_addr = attr->demod_addr; //0x20;
	demod_ct_cfg.ts_out_mode = (attr->output_mode < MT_UNF_FE_OUTPUT_MODE_SERIAL) ? MtFeTsOutMode_Parallel : MtFeTsOutMode_Serial;
	demod_ct_cfg.ts_out_swap = MtFeTsOut_Normal;

	demod_dc_cfg.demod_mode = MtFeType_DVBC;
	demod_dc_cfg.demod_bandwidth = MtFeBandwidth_8M;
	demod_dc_cfg.demod_i2c_addr = attr->demod_addr; //0x20;//0x38;
	demod_dc_cfg.ts_out_mode = (attr->output_mode < MT_UNF_FE_OUTPUT_MODE_SERIAL) ? MtFeTsOutMode_Parallel : MtFeTsOutMode_Serial;
	demod_dc_cfg.ts_out_swap = MtFeTsOut_Normal;

	printk("%s[%d] -- output_mode = %d - %d\n", __FUNCTION__, __LINE__, attr->output_mode, demod_dc_cfg.ts_out_mode);

	if (attr->tuner_type == MT_UNF_TUNER_TYPE_M88TC3800)
	{
		tuner_ct_cfg.tuner_type = MtFeTN_TC3800;
		tuner_ct_cfg.tuner_mode = MtFeType_CTTB;
		tuner_ct_cfg.tuner_dev_addr = attr->tuner_addr; //0xC2;
		tuner_ct_cfg.tuner_init_ok = 0;
		tuner_ct_cfg.tuner_init = (S32 (*)(void *))mt_fe_tn_init_tc3800_tc_dd3k;
		tuner_ct_cfg.tuner_set = (S32 (*)(void *, U32))mt_fe_tn_set_freq_tc3800_tc_dd3k;
		tuner_ct_cfg.tuner_get_strength = (S32 (*)(void *, S8 *))mt_fe_tn_get_strength_tc3800_tc_dd3k;

		tuner_dc_cfg.tuner_type = MtFeTN_TC3800;
		tuner_dc_cfg.tuner_mode = MtFeType_DVBC;
		tuner_dc_cfg.tuner_dev_addr = attr->tuner_addr; //0xC2;
		tuner_dc_cfg.tuner_init_ok = 0;
		tuner_dc_cfg.tuner_init = (S32 (*)(void *))mt_fe_tn_init_tc3800_tc_dd3k;
		tuner_dc_cfg.tuner_set = (S32 (*)(void *, U32))mt_fe_tn_set_freq_tc3800_tc_dd3k;
		tuner_dc_cfg.tuner_get_strength = (S32 (*)(void *, S8 *))mt_fe_tn_get_strength_tc3800_tc_dd3k;

		//printk("----------------dd3k select ctt2 tuner TC3800-------------------------------\n");
	}
	else if (attr->tuner_type == MT_UNF_TUNER_TYPE_MXL603)
	{
		printk("[%s %d]MT_UNF_TUNER_TYPE_MXL603\n", __FUNCTION__, __LINE__);
		tuner_ct_cfg.tuner_type = MtFeTN_MxL603;
		tuner_ct_cfg.tuner_mode = MtFeType_CTTB;
		tuner_ct_cfg.tuner_dev_addr = attr->tuner_addr;//0xC6;
		tuner_ct_cfg.tuner_init_ok = 0;
		tuner_ct_cfg.tuner_init = (S32 (*)(void *))mt_fe_tn_init_MxL603_dd3k;
		tuner_ct_cfg.tuner_set = (S32 (*)(void *, U32))mt_fe_tn_set_freq_MxL603_dd3k;
		tuner_ct_cfg.tuner_get_strength = (S32 (*)(void *, S8 *))mt_fe_tn_get_strength_MxL603_dd3k;
		tuner_ct_cfg.tuner_sleep = (S32 (*)(void *))mt_fe_tn_sleep_MxL603_dd3k;
		tuner_ct_cfg.tuner_wake_up = (S32 (*)(void *))mt_fe_tn_wake_up_MxL603_dd3k;

		tuner_dc_cfg.tuner_type = MtFeTN_MxL603;
		tuner_dc_cfg.tuner_mode = MtFeType_DVBC;
		tuner_dc_cfg.tuner_dev_addr = attr->tuner_addr;//0xC6;
		tuner_dc_cfg.tuner_init_ok = 0;
		tuner_dc_cfg.tuner_init = (S32 (*)(void *))mt_fe_tn_init_MxL603_dd3k;
		tuner_dc_cfg.tuner_set = (S32 (*)(void *, U32))mt_fe_tn_set_freq_MxL603_dd3k;
		tuner_dc_cfg.tuner_get_strength = (S32 (*)(void *, S8 *))mt_fe_tn_get_strength_MxL603_dd3k;
		tuner_dc_cfg.tuner_sleep = (S32 (*)(void *))mt_fe_tn_sleep_MxL603_dd3k;
		tuner_dc_cfg.tuner_wake_up = (S32 (*)(void *))mt_fe_tn_wake_up_MxL603_dd3k;

		//printk("----------------dd3k select tuner MxL603-------------------------------\n");
	}
	else if (attr->tuner_type == MT_UNF_TUNER_TYPE_MXL608)
	{
		printk("[%s %d]MT_UNF_TUNER_TYPE_MXL608\n", __FUNCTION__, __LINE__);
		tuner_ct_cfg.tuner_type = MtFeTN_MxL608;
		tuner_ct_cfg.tuner_mode = MtFeType_CTTB;
		tuner_ct_cfg.tuner_dev_addr = attr->tuner_addr;//0xC6;
		tuner_ct_cfg.tuner_init_ok = 0;
		tuner_ct_cfg.tuner_init = (S32 (*)(void *))mt_fe_tn_init_MxL608_dd3k;
		tuner_ct_cfg.tuner_set = (S32 (*)(void *, U32))mt_fe_tn_set_freq_MxL608_dd3k;
		tuner_ct_cfg.tuner_get_strength = (S32 (*)(void *, S8 *))mt_fe_tn_get_strength_MxL608_dd3k;
		tuner_ct_cfg.tuner_sleep = (S32 (*)(void *))mt_fe_tn_sleep_MxL608_dd3k;
		tuner_ct_cfg.tuner_wake_up = (S32 (*)(void *))mt_fe_tn_wake_up_MxL608_dd3k;

		tuner_dc_cfg.tuner_type = MtFeTN_MxL608;
		tuner_dc_cfg.tuner_mode = MtFeType_DVBC;
		tuner_dc_cfg.tuner_dev_addr = attr->tuner_addr;//0xC6;
		tuner_dc_cfg.tuner_init_ok = 0;
		tuner_dc_cfg.tuner_init = (S32 (*)(void *))mt_fe_tn_init_MxL608_dd3k;
		tuner_dc_cfg.tuner_set = (S32 (*)(void *, U32))mt_fe_tn_set_freq_MxL608_dd3k;
		tuner_dc_cfg.tuner_get_strength = (S32 (*)(void *, S8 *))mt_fe_tn_get_strength_MxL608_dd3k;
		tuner_dc_cfg.tuner_sleep = (S32 (*)(void *))mt_fe_tn_sleep_MxL608_dd3k;
		tuner_dc_cfg.tuner_wake_up = (S32 (*)(void *))mt_fe_tn_wake_up_MxL608_dd3k;

		//printk("----------------dd3k select tuner MxL603-------------------------------\n");
	}
	else// if (attr->tuner_type == MT_UNF_TUNER_TYPE_M88TC6800)
	{
		tuner_ct_cfg.tuner_type = MtFeTN_TC6800;
		tuner_ct_cfg.tuner_mode = MtFeType_CTTB;
		tuner_ct_cfg.tuner_dev_addr = attr->tuner_addr; //0xC6;
		tuner_ct_cfg.tuner_init_ok = 0;
		tuner_ct_cfg.tuner_init = (S32 (*)(void *))mt_fe_tn_init_tc6800_tc_dd3k;
		tuner_ct_cfg.tuner_set = (S32 (*)(void *, U32))mt_fe_tn_set_freq_tc6800_tc_dd3k;
		tuner_ct_cfg.tuner_get_strength = (S32 (*)(void *, S8 *))mt_fe_tn_get_strength_tc6800_tc_dd3k;

		tuner_dc_cfg.tuner_type = MtFeTN_TC6800;
		tuner_dc_cfg.tuner_mode = MtFeType_DVBC;
		tuner_dc_cfg.tuner_dev_addr = attr->tuner_addr; //0xC6;
		tuner_dc_cfg.tuner_init_ok = 0;
		tuner_dc_cfg.tuner_init = (S32 (*)(void *))mt_fe_tn_init_tc6800_tc_dd3k;
		tuner_dc_cfg.tuner_set = (S32 (*)(void *, U32))mt_fe_tn_set_freq_tc6800_tc_dd3k;
		tuner_dc_cfg.tuner_get_strength = (S32 (*)(void *, S8 *))mt_fe_tn_get_strength_tc6800_tc_dd3k;

		//printk("----------------dd3k select ctt2 tuner TC6800-------------------------------\n");
	}

	//mt_fe_config_tuner_settings_dd3k(dev_handle, );

	dev_handle->dmd_set_reg = (MT_FE_RET (*)(void *, U16, U8))_mt_fe_dmd_set_reg_dd3k;
	dev_handle->dmd_get_reg = (MT_FE_RET (*)(void *, U16, U8 *))_mt_fe_dmd_get_reg_dd3k;
	dev_handle->write_fw = (MT_FE_RET (*)(void *, U16, U8 *, U16))_mt_fe_dmd_write;
	//dev_handle->tn_set_reg = (S32 (*)(void *, U8, U8))fe_dd3k_tn_set_reg;
	//dev_handle->tn_get_reg = (S32 (*)(void *, U8, U8 *))fe_dd3k_tn_get_reg;
	dev_handle->tn_write = (MT_FE_RET (*)(void *, U8 *, U16))_mt_fe_tn_write_dd3k;
	dev_handle->tn_read = (MT_FE_RET (*)(void *, U8 *, U16, U8 *, U16))_mt_fe_tn_read_dd3k;
	dev_handle->mt_sleep = (void (*)(U32))_mt_sleep_dd3k;

	mt_fe_config_demod_settings_dd3k(dev_handle, &demod_ct_cfg);
	mt_fe_config_demod_settings_dd3k(dev_handle, &demod_dc_cfg);
	mt_fe_config_tuner_settings_dd3k(dev_handle, &tuner_ct_cfg);
	mt_fe_config_tuner_settings_dd3k(dev_handle, &tuner_dc_cfg);

	mt_fe_config_scan_settings_dd3k_c(dev_handle, &dc_qam_cfg);

	mt_fe_dmd_init_dd3k(dev_handle);

	info->is_attach = 1;

	return MT_SUCCESS;
}

int m88dd3k_detach(frontend_info_s *info)
{
	mt_fe_dd3k_priv_handle p_priv = NULL;
	MT_FE_DD_Device_Handle dev_handle = NULL;

	if (info->is_attach)
	{
		p_priv = info->handle;
		dev_handle = p_priv->dd3k_handle;

		kfree(dev_handle);
		kfree((void *)p_priv);

		info->is_attach = 0;
	}

	g_dd3k_priv = NULL;

	return MT_SUCCESS;
}
#endif

