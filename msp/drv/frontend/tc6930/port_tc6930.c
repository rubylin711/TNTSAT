/********************************************************************************************/
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/*****************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd                                    */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2016 Montage Technology Group Limited. All Rights Reserved. */
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

#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

#include <net/sock.h>
#include <net/genetlink.h>

#include "mt_unf_frontend.h"
#include "mt_fe_common_tc6930.h"
#include "port_tc6930.h"
#include "drv_frontend_ioctl.h"

#include "mt_fe_i2c_tc6930.h"

#include "mt_module_debug.h"

#include "mt_mach/chipinfo.h"



static U8 gDemodIndex = 0;


mt_fe_tc6930_priv_handle g_tc6930_priv = NULL;
int g_i2c_tc6930 = 0;

static mt_fe_tc6930_priv_handle p_priv = NULL;
static MT_FE_TC6930_Device_Handle dev_handle = NULL;


static int port_m88tc6930_channel_set(void *handle, mt_unf_fe_connect_para_t *para)
{
	//MT_U32 for_scan = 0;
	MT_FE_RET ret = 0;
	MT_FE_TYPE dvb_type = MtFeType_Undef;
	mt_unf_fe_channel_info_t *p_channel_info = NULL;
	mt_fe_tc6930_priv_handle p_priv = (mt_fe_tc6930_priv_handle)handle;
	MT_FE_TC6930_Device_Handle tc6930_handle = p_priv->tc6930_handle;

	U32 uiFreqKHz = 850000;
	U16 usSymRateKSs = 6875;
	U16 usQam = 0;
	U8 ucInverted = 0;

	p_channel_info = &(para->channel_info);
	para->channel_info.lock = 0;

	dvb_type = MtFeType_DVBC;

	memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));

	uiFreqKHz = para->connect_param.cab.freq;
	usSymRateKSs = para->connect_param.cab.sym_rate / 1000;
	ucInverted = para->connect_param.cab.b_reverse;

	switch (para->connect_param.cab.mod_type)
	{
		case MT_UNF_MOD_TYPE_QAM_16:
			usQam = 16;
			break;

		case MT_UNF_MOD_TYPE_QAM_32:
			usQam = 32;
			break;

		case MT_UNF_MOD_TYPE_QAM_64:
			usQam = 64;
			break;

		case MT_UNF_MOD_TYPE_QAM_128:
			usQam = 128;
			break;

		case MT_UNF_MOD_TYPE_QAM_256:
			usQam = 256;
			break;

		default:
			usQam = 0;
			break;
	}

	printk("%s[%d] -- set c: feq %d, symbol rate = %d, qam = %d, type = %d\n",
		   __FUNCTION__, __LINE__,
		   para->connect_param.cab.freq,
		   para->connect_param.cab.sym_rate,
		   usQam,
		   dvb_type);

	ret = mt_fe_dmd_tc6930_connect(tc6930_handle, gDemodIndex, uiFreqKHz, usSymRateKSs, usQam, ucInverted);

	para->channel_set_info.lock_time = 800;

	if (ret == MtFeErr_Ok)
	{
		return MT_SUCCESS;
	}

	return MT_FAILURE;
}

static int port_m88tc6930_get_status(void *handle, mt_unf_fe_status_t *p_status)
{
	mt_fe_tc6930_priv_handle p_priv = (mt_fe_tc6930_priv_handle)handle;
	MT_FE_TC6930_Device_Handle tc6930_handle = p_priv->tc6930_handle;
	MT_FE_LOCK_STATE stat = 0;
	//U8 tmp;

	mt_fe_dmd_tc6930_get_lock_state(tc6930_handle, gDemodIndex, &stat);

	if (stat == MtFeLockState_Locked)
	{
		p_status->lock_status = MT_UNF_FE_SIGNAL_LOCKED;
	}
	else
	{
		p_status->lock_status = MT_UNF_FE_SIGNAL_DROPPED;
		if (stat == MtFeLockState_Unlocked)
			p_status->unlock_reason = MT_UNF_FE_STATE_UNLOCKED;
		else
			p_status->unlock_reason = MT_UNF_FE_STATE_WAITING;
	}

	//tc6930_handle->dmd_get_reg(tc6930_handle, gDemodIndex, 0x85, &tmp);
	//printk("%s[%d] -- 0x85 = 0x%02x, %s!\n", __FUNCTION__, __LINE__, tmp, (stat == MtFeLockState_Locked) ? "Locked" : "Unlock");

	return MT_SUCCESS;
}

static int port_m88tc6930_get_signal_quality(void *handle, MT_U32 *p_quality)
{
	mt_fe_tc6930_priv_handle p_priv = (mt_fe_tc6930_priv_handle)handle;
	MT_FE_TC6930_Device_Handle tc6930_handle = p_priv->tc6930_handle;
	int ret = 0;
	U8 percent = 0;//, snr = 0;

	//ret = mt_fe_dmd_tc6930_get_snr(tc6930_handle, gDemodIndex, &snr);
	mt_fe_dmd_tc6930_get_quality(tc6930_handle, gDemodIndex, &percent);

	if (ret < 0)
		return MT_FAILURE;

	*p_quality = percent;

	return MT_SUCCESS;
}

static int port_m88tc6930_get_ber(void *handle, MT_U32 *p_ber)
{
	mt_fe_tc6930_priv_handle p_priv = (mt_fe_tc6930_priv_handle)handle;
	MT_FE_TC6930_Device_Handle tc6930_handle = p_priv->tc6930_handle;
	U32 err_packages = 0;
	U32 total_packages = 0;

	mt_fe_dmd_tc6930_get_ber(tc6930_handle, gDemodIndex, &err_packages, &total_packages);

	p_ber[0] = total_packages;
	p_ber[1] = err_packages;
	p_ber[2] = 0;

	return MT_SUCCESS;
}
static int __maybe_unused port_m88tc6930_get_per(void *handle, MT_U32 *p_per)
{
	mt_fe_tc6930_priv_handle p_priv = (mt_fe_tc6930_priv_handle)handle;
	MT_FE_TC6930_Device_Handle tc6930_handle = p_priv->tc6930_handle;
	U32 err_packages = 0;
	U32 total_packages = 0;

	mt_fe_dmd_tc6930_get_per(tc6930_handle, gDemodIndex, &err_packages, &total_packages);

	p_per[0] = total_packages;
	p_per[1] = err_packages;
	p_per[2] = 0;

	return MT_SUCCESS;
}
static int port_m88tc6930_get_snr(void *handle, MT_U32 *p_snr)
{
	mt_fe_tc6930_priv_handle p_priv = (mt_fe_tc6930_priv_handle)handle;
	MT_FE_TC6930_Device_Handle tc6930_handle = p_priv->tc6930_handle;
	U8 _snr = 0;
	int ret = 0;

	ret = mt_fe_dmd_tc6930_get_snr(tc6930_handle, gDemodIndex, &_snr);
	if (ret < 0)
		return MT_FAILURE;

	*p_snr = _snr;

	return MT_SUCCESS;
}

static int port_m88tc6930_get_signal_strength(void *handle, MT_U32 *p_strength)
{
	mt_fe_tc6930_priv_handle p_priv = (mt_fe_tc6930_priv_handle)handle;
	MT_FE_TC6930_Device_Handle tc6930_handle = p_priv->tc6930_handle;
	S8 _strength = 0;
	int ret = 0;

	ret = mt_fe_dmd_tc6930_get_strength(tc6930_handle, gDemodIndex, &_strength);
	if (ret < 0)
		return MT_FAILURE;

	*p_strength = _strength;

	return MT_SUCCESS;
}

#if 0
static void port_m88tc6930_get_signal_info(void *handle, mt_unf_fe_signal_info_t *p_sig_info)
{
	mt_fe_tc6930_priv_handle p_priv = (mt_fe_tc6930_priv_handle)handle;
	MT_FE_TC6930_Device_Handle tc6930_handle = p_priv->tc6930_handle;

	p_sig_info->sig_type = MT_UNF_FE_SIG_TYPE_CAB;

	p_sig_info->sig_info.cab.cab_type	 = MT_UNF_FE_DVBC;
	p_sig_info->sig_info.cab.freq		 = tc6930_handle->demod_device[gDemodIndex].channel_param.freq_KHz;
	p_sig_info->sig_info.cab.symbol_rate = tc6930_handle->demod_device[gDemodIndex].channel_param.sym_KSs;
	p_sig_info->sig_info.cab.iq_mode	 = tc6930_handle->demod_device[gDemodIndex].channel_param.inverted;

	switch(tc6930_handle->demod_device[gDemodIndex].channel_param.qam_code)
	{
		case  16:	p_sig_info->sig_info.cab.mode_type = MT_UNF_MOD_TYPE_QAM_16;		break;
		case  32:	p_sig_info->sig_info.cab.mode_type = MT_UNF_MOD_TYPE_QAM_32;		break;
		case  64:	p_sig_info->sig_info.cab.mode_type = MT_UNF_MOD_TYPE_QAM_64;		break;
		case 128:	p_sig_info->sig_info.cab.mode_type = MT_UNF_MOD_TYPE_QAM_128;		break;
		case 256:	p_sig_info->sig_info.cab.mode_type = MT_UNF_MOD_TYPE_QAM_256;		break;
		default:	p_sig_info->sig_info.cab.mode_type = MT_UNF_MOD_TYPE_AUTO;			break;
	}

	return;
}
#endif

static int port_m88tc6930_get_signal_agc(void *handle, mt_u32 *p_agc)
{
	mt_fe_tc6930_priv_handle p_priv = (mt_fe_tc6930_priv_handle)handle;
	MT_FE_TC6930_Device_Handle tc6930_handle = p_priv->tc6930_handle;
	MT_S32 tuner_level = 0;

	tuner_level = mt_fe_tn_get_signal_strength_tc6930(tc6930_handle, gDemodIndex);

	tuner_level += 107;
	if (tuner_level < 0)
		tuner_level = 0;

	*p_agc = tuner_level;

	//printk("%s[%d] -- signal level = %d dBuV\n", __FUNCTION__, __LINE__, tuner_level);

	return MT_SUCCESS;
}

static int port_m88tc6930_standby(void *handle)
{
	mt_fe_tc6930_priv_handle p_priv = (mt_fe_tc6930_priv_handle)handle;
	MT_FE_TC6930_Device_Handle tc6930_handle = p_priv->tc6930_handle;

	//mt_fe_tn_sleep_tc6930(tc6930_handle, gDemodIndex);
	mt_fe_tn_sleep_all_tc6930(tc6930_handle);

	return MT_SUCCESS;
}

static int port_m88tc6930_wakeup(void *handle)
{
	mt_fe_tc6930_priv_handle p_priv = (mt_fe_tc6930_priv_handle)handle;
	MT_FE_TC6930_Device_Handle tc6930_handle = p_priv->tc6930_handle;

	mt_fe_tn_wakeup_tc6930(tc6930_handle, gDemodIndex);

	return MT_SUCCESS;
}

static int port_m88tc6930_get_default_timeout(void *handle, mt_u32 *timeout)
{
	*timeout = 800;

	return MT_SUCCESS;
}

static int port_m88tc6930_set_io(void *handle, MT_BOOL onoff)
{
	return MT_SUCCESS;
}

static int port_m88tc6930_channel_connect(void *handle, mt_unf_fe_connect_para_t *para)
{
	mt_unf_fe_channel_info_t *p_channel_info = NULL;
	mt_fe_tc6930_priv_handle p_priv = (mt_fe_tc6930_priv_handle)handle;
	MT_FE_TC6930_Device_Handle tc6930_handle = p_priv->tc6930_handle;

	MT_FE_LOCK_STATE status = 0;
	MT_U32 cnt = 0;
	//U8 tmp = 0;
	//int i = 0;

	//U8 strength = 0;

	p_priv->sig_type = para->sig_type;
	p_channel_info = &(para->channel_info);
	//pinmux_configure();

	//printk("----port_m88tc6930_channel_connect[%d] log1, sig_type = %d\n", gDemodIndex, para->sig_type);

#if 0
	printk("tc6930_channel set c: feq %d, symbol rate = %d, type = %d\n",
			para->connect_param.cab.freq,
			para->connect_param.cab.sym_rate,
			dvb_type);
#endif

	memcpy(&p_priv->cur_channel, p_channel_info, sizeof(mt_unf_fe_channel_info_t));
	para->channel_info.lock = 0;

	port_m88tc6930_channel_set(p_priv, para);

	//printk("lock_timeout_ms :%d\n", para->stChannelSetInfo.lock_time);
	for (cnt = 0; cnt < para->channel_set_info.lock_time; cnt += 10)
	{
		mt_fe_dmd_tc6930_get_lock_state(tc6930_handle, gDemodIndex, &status);

		//tc6930_handle->dmd_get_reg(tc6930_handle, gDemodIndex, 0x85, &tmp);
		//printk("%s[%d] -- Demod %d, 0x85 = [%02x]\n", __FUNCTION__, __LINE__, gDemodIndex, tmp);

		para->channel_info.lock = (status == MtFeLockState_Locked) ? 1 : 0;
		if (para->channel_info.lock)
		{
			//mt_fe_dmd_tc6930_get_strength(tc6930_handle, gDemodIndex, &strength);
			//printk("tc6930 connect[%d] lock success, strength = %d\n", gDemodIndex, strength);
			return MT_SUCCESS;
		}
		//mdelay(10);
		msleep(10);
	}

#if 0
	for(i = 0; i < 0x100; i ++)
	{
		tc6930_handle->dmd_get_reg(tc6930_handle, gDemodIndex, (U8)i, &tmp);

		printk("\t%d,\t%02x - %02x\n", gDemodIndex, (U8)i, tmp);
	}

	printk("\n");
#else

#if 0
	mt_fe_dmd_tc6930_get_strength(tc6930_handle, gDemodIndex, &strength);

	printk("tc6930 connect[%d] lock failed, strength = %d\n", gDemodIndex, strength);

	if (gDemodIndex == 2)
	{
		U8 val = 0;
		int i = 0;

		for (i = 0; i < 0x100; i++)
		{
			tc6930_handle->dmd_get_reg(tc6930_handle, gDemodIndex, (U8)i, &val);
			printk("\t%d,\t%02x - %02x\n", gDemodIndex, (U8)i, val);
		}
	}
#endif

#endif

	return ERR_TIMEOUT;
}

//static int port_m88tc6930_ioctl(void *handle, mt_u32 cmd, mt_u32 param)
static int port_m88tc6930_ioctl(void *handle, mt_u32 cmd, ulong param)
{
	MT_FE_LOCK_STATE status = 0;

	mt_fe_tc6930_priv_handle p_priv = (mt_fe_tc6930_priv_handle)handle;
	MT_FE_TC6930_Device_Handle tc6930_handle = p_priv->tc6930_handle;

	switch (cmd)
	{
		case NIM_IOCTRL_CHANNEL_CHECK_LOCK:
			mt_fe_dmd_tc6930_get_lock_state(tc6930_handle, gDemodIndex, &status);

			if (param != 0)
				((mt_unf_fe_channel_info_t *)param)->lock = (status == MtFeLockState_Locked) ? 1 : 0;
			break;

		case NIM_IOCTRL_SET_MODULE_INDEX:
#if 0
			if (param == 2)
			{
				gDemodIndex = 1;
			}
			else if (param = 1)
			{
				gDemodIndex = 0;
			}
			else
			{
				gDemodIndex = 2;
			}
#else
			gDemodIndex = param;

			if (gDemodIndex > 2)
				gDemodIndex = 0;
#endif
			tc6930_handle->cur_demod_index = gDemodIndex;
			//printk("%s[%d]: set current demod index %d\n", __FUNCTION__, __LINE__, gDemodIndex);
			break;

		default:
			break;
	}

	return MT_SUCCESS;
}

#ifdef CONFIG_NET
int m88tc6930_attach(frontend_info_s *info, mt_unf_fe_attr_t *attr)
{
	//int i = 0;
	//U8 tmp = 0;
	U32 ulTmp = 0;
	mt_u32 temp = 0;

	info->ops.connect = port_m88tc6930_channel_connect;
	info->ops.get_status = port_m88tc6930_get_status;
	info->ops.get_ber = port_m88tc6930_get_ber;
	info->ops.get_snr = port_m88tc6930_get_snr;
	info->ops.get_signal_strength = port_m88tc6930_get_signal_strength;
	info->ops.get_signal_quality = port_m88tc6930_get_signal_quality;
	info->ops.get_signal_agc = port_m88tc6930_get_signal_agc;

	info->ops.standby = port_m88tc6930_standby;
	info->ops.wakeup = port_m88tc6930_wakeup;
	info->ops.get_default_timeout = port_m88tc6930_get_default_timeout;
	info->ops.set_io = port_m88tc6930_set_io;
	info->ops.port_ioctl = port_m88tc6930_ioctl;
	info->ops.blind_scan = NULL;

	if (p_priv == NULL)
	{
		p_priv = kzalloc(sizeof(mt_fe_tc6930_priv_t), GFP_KERNEL);
		if (p_priv == NULL)
			return -ENOMEM;
	}

	if (dev_handle == NULL)
	{
		dev_handle = kzalloc(sizeof(MT_FE_TC6930_SETTINGS), GFP_KERNEL);
		if (dev_handle == NULL)
		{
			kfree((void *)p_priv);
			g_tc6930_priv = NULL;
			return -ENOMEM;
		}
	}

	p_priv->tc6930_handle = dev_handle;
	info->handle = (void *)p_priv;
	g_tc6930_priv = p_priv;

	g_i2c_tc6930 = attr->demod_i2c_id;

	memcpy(&(p_priv->cfg), &(attr->fe_config), sizeof(mt_unf_fe_config_para_t));

	mt_fe_dmd_tc6930_config_default(dev_handle, 0);

	if (attr->demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88TC6920)
	{
		dev_handle->demod_device[0].tuner_settings.tuner_index = 1;
		dev_handle->demod_device[1].tuner_settings.tuner_index = 0;
		dev_handle->demod_device[2].tuner_settings.tuner_index = 2;

		dev_handle->demod_device[0].demod_type = MT_FE_DEMOD_TC6920;
		dev_handle->demod_device[1].demod_type = MT_FE_DEMOD_TC6920;
	}
	else if (attr->demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88RC6800)
	{
		dev_handle->demod_device[0].tuner_settings.tuner_index = 1;
		dev_handle->demod_device[1].tuner_settings.tuner_index = 0;
		dev_handle->demod_device[2].tuner_settings.tuner_index = 2;

		dev_handle->demod_device[0].demod_type = MT_FE_DEMOD_RC6800;
		dev_handle->demod_device[1].demod_type = MT_FE_DEMOD_RC6800;
	}
	else if (attr->demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88TC6930)
	{
		dev_handle->demod_device[0].tuner_settings.tuner_index = 1;
		dev_handle->demod_device[1].tuner_settings.tuner_index = 2;
		dev_handle->demod_device[2].tuner_settings.tuner_index = 0;

		dev_handle->demod_device[0].demod_type = MT_FE_DEMOD_TC6930;
		dev_handle->demod_device[1].demod_type = MT_FE_DEMOD_TC6930;
	}
	else //if (attr->demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88TC6960)
	{
		dev_handle->demod_device[0].tuner_settings.tuner_index = 1;
		dev_handle->demod_device[1].tuner_settings.tuner_index = 2;
		dev_handle->demod_device[2].tuner_settings.tuner_index = 0;

		dev_handle->demod_device[0].demod_type = MT_FE_DEMOD_TC6960;
		dev_handle->demod_device[1].demod_type = MT_FE_DEMOD_TC6960;
	}

	mt_fe_dmd_tc6930_match_tuners(dev_handle);

	switch (symphony_get_chip_rev())
	{
		case CHIP_SYMPHONY_A0:
		case CHIP_SYMPHONY_A1:
		case CHIP_SYMPHONY_A2:
		case CHIP_SYMPHONY3_A0:
			dev_handle->demod_device[2].demod_type		 = MT_FE_DEMOD_CS8000_CAB;
			dev_handle->demod_device[2].demod_dev_addr	 = 0x38;

			printk("%s[%d] -- Symphony1 or Symphony3, demod_type = %d\n", __FUNCTION__, __LINE__, dev_handle->demod_device[2].demod_type);
			break;

		case CHIP_SYMPHONY2_A0:
		case CHIP_SYMPHONY2_A1:
		case CHIP_SYMPHONY2_A2:
		case CHIP_SYMPHONY2_A3:
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

			dev_handle->demod_device[2].demod_type = MT_FE_DEMOD_CT8000;
			printk("%s[%d] -- Symphony2, demod_type = %d\n", __FUNCTION__, __LINE__, dev_handle->demod_device[2].demod_type);

			//BF13C00C [31:28] = 1, BF13C010 [11:8] = 1
			dev_handle->Get32Bits(0xBF13C00C, &ulTmp);
			ulTmp &= 0x0FFFFFFF;
			ulTmp |= 0x10000000;
			dev_handle->Set32Bits(0xBF13C00C, ulTmp);

			dev_handle->Get32Bits(0xBF13C010, &ulTmp);
			ulTmp &= 0xFFFFF0FF;
			ulTmp |= 0x00000100;
			dev_handle->Set32Bits(0xBF13C010, ulTmp);

#if 0	// TS 3
			//sw_pin2_sel[23:16] = 0x44
			dev_handle->Get32Bits(0xBF13C008, &ulTmp);
			ulTmp &= 0xFF00FFFF;
			ulTmp |= 0x00440000;
			dev_handle->Set32Bits(0xBF13C008, ulTmp);

			//BF138008 bit20 = 0
			dev_handle->Get32Bits(0xBF138008, &ulTmp);
			ulTmp &= ~0x100000;
			dev_handle->Set32Bits(0xBF138008, ulTmp);
#else	// TS 0
			//sw_pin2_sel[15:12] = 0x0
			//sw_pin2_sel[27:24] = 0x0
			dev_handle->Get32Bits(0xBF13C008, &ulTmp);
			ulTmp &= 0xF0FF0FFF;
			ulTmp &= 0xF0000FFF;
			dev_handle->Set32Bits(0xBF13C008, ulTmp);

			//BF138008 bit12 = 0
			dev_handle->Get32Bits(0xBF138008, &ulTmp);
			ulTmp &= ~0x1000;
			dev_handle->Set32Bits(0xBF138008, ulTmp);

			//BF500024 bit4 = 1
			dev_handle->Get32Bits(0xBF500024, &ulTmp);
			ulTmp |= 0x10;
			dev_handle->Set32Bits(0xBF500024, ulTmp);
#endif

			break;

		case CHIP_SYMPHONY4_A0:
		case CHIP_SYMPHONY4_A1:
			dev_handle->demod_device[2].demod_type = MT_FE_DEMOD_CS8800;
			printk("%s[%d] -- Symphony4, demod_type = %d\n", __FUNCTION__, __LINE__, dev_handle->demod_device[2].demod_type);

			dev_handle->demod_device[2].ts_output_settings.output_mode = MtFeTsOutMode_Parallel;

			//1.	配置0xbf50f818[18]， 配置值为0x1    //将apb clk3的时钟切换到xtal
			dev_handle->Get32Bits(0xbf50f818, &temp);
			temp |= 0x40000;
			dev_handle->Set32Bits(0xbf50f818, temp);	//+++ CRM: TOPCLK_CTRL6_REG


			//2.	等待1us
			//_mt_delayus_cs8800(10);
			//mdelay(1);
			usleep_range(500, 1000);

			//3.	配置0xbf508004[7:6]，配置值为0x3  //将apb 时钟切换到 apb clk3，即xtal时钟
			dev_handle->Get32Bits(0xbf508004, &temp);
			temp |= 0xc0;
			dev_handle->Set32Bits(0xbf508004, temp);	//+++ CRM: BUS_CLKSEL_REG

			//4.	等待1us
			//_mt_delayus_cs8800(10);
			//mdelay(1);
			usleep_range(500, 1000);

			//5.	配置0xbf50f818[24]，配置值为0x1  //使能demo时钟
			dev_handle->Get32Bits(0xbf50f818, &temp);
			temp |= 0x1000000;
			dev_handle->Set32Bits(0xbf50f818, temp);	//+++ CRM: TOPCLK_CTRL6_REG

			//6.	等待1us
			//_mt_delayus_cs8800(10);
			//mdelay(1);
			usleep_range(500, 1000);

			//7.	配置0xbf508004[7:6]，配置值为0x0 //将apb 时钟切换到 apb clk0，即80MHz时钟
			dev_handle->Get32Bits(0xbf508004, &temp);
			temp &= ~0xc0;
			dev_handle->Set32Bits(0xbf508004, temp);	//+++ CRM: BUS_CLKSEL_REG


			/*read 0xbf5b0c00 before demod start working*/

			//_mt_fe_read32_cs8800(0xbf5b0c00, &temp);
			HAL_GET_U8((volatile u8 *)(mt_get_demod_base()+ 0x00));

#if 0

			//BF13C038[3:0]=0 //TS0_DATA
			dev_handle->Get32Bits(0xBF13C038, &ulTmp);
			ulTmp &= 0xFFFFFFF0;
			dev_handle->Set32Bits(0xBF13C038, ulTmp);

			//BF13C03C[3:0]=0 //TS0_VALID
			dev_handle->Get32Bits(0xBF13C03C, &ulTmp);
			ulTmp &= 0xFFFFFFF0;
			dev_handle->Set32Bits(0xBF13C03C, ulTmp);

			//BF13C040[3:0]=0 //TS0_SYNC
			dev_handle->Get32Bits(0xBF13C040, &ulTmp);
			ulTmp &= 0xFFFFFFF0;
			dev_handle->Set32Bits(0xBF13C040, ulTmp);

			//BF13C044[3:0]=0 //TS0_CLK
			dev_handle->Get32Bits(0xBF13C044, &ulTmp);
			ulTmp &= 0xFFFFFFF0;
			dev_handle->Set32Bits(0xBF13C044, ulTmp);

			//BF50B000[0]=1 //打开TSI时钟
			dev_handle->Get32Bits(0xBF50B000, &ulTmp);
			ulTmp |= 0x01;
			dev_handle->Set32Bits(0xBF50B000, ulTmp);

			//BF50B004[2]=0/ ts0 clk 正沿
			dev_handle->Get32Bits(0xBF50B004, &ulTmp);
			ulTmp &= 0xFFFFFFFE;
			dev_handle->Set32Bits(0xBF50B004, ulTmp);

			//BF50B00C[3]=1/ ts0复位释放
			dev_handle->Get32Bits(0xBF50B00C, &ulTmp);
			ulTmp |= 0x08;
			dev_handle->Set32Bits(0xBF50B00C, ulTmp);

			//BF200000[31:0]=0xC7E101FF//TC0 CTR
			dev_handle->Set32Bits(0xBF200000, 0xC7E101FF);
#endif

#if 1
			//BF5D0094[0]=1  //IQ PAD设置为digital input
			dev_handle->Get32Bits(0xBF5D0094, &ulTmp);
			ulTmp |= 0x01;
			dev_handle->Set32Bits(0xBF5D0094, ulTmp);

			//BF13C190[2:0]=2 // TS1_CLK
			dev_handle->Get32Bits(0xBF13C190, &ulTmp);
			ulTmp &= 0xFFFFFFF8;
			ulTmp |= 0x02;
			dev_handle->Set32Bits(0xBF13C190, ulTmp);

			//BF13C194[2:0]=1 // TS1_SYNC
			dev_handle->Get32Bits(0xBF13C194, &ulTmp);
			ulTmp &= 0xFFFFFFF8;
			ulTmp |= 0x01;
			dev_handle->Set32Bits(0xBF13C194, ulTmp);

			//BF13C198[2:0]=1 // TS1_VALID
			dev_handle->Get32Bits(0xBF13C198, &ulTmp);
			ulTmp &= 0xFFFFFFF8;
			ulTmp |= 0x01;
			dev_handle->Set32Bits(0xBF13C198, ulTmp);

			//BF13C19C[2:0]=2 // TS1_DATA
			dev_handle->Get32Bits(0xBF13C19C, &ulTmp);
			ulTmp &= 0xFFFFFFF8;
			ulTmp |= 0x02;
			dev_handle->Set32Bits(0xBF13C19C, ulTmp);

			//BF138008[4]=0 //TS1 input
			dev_handle->Get32Bits(0xBF138008, &ulTmp);
			ulTmp &= 0xFFFFFFEF;
			dev_handle->Set32Bits(0xBF138008, ulTmp);


			//BF50B000[0]=1 //打开时钟
			dev_handle->Get32Bits(0xBF50B000, &ulTmp);
			ulTmp |= 0x01;
			dev_handle->Set32Bits(0xBF50B000, ulTmp);

			//BF50B004[4:3]= 10 //ts1 clk 正沿
			dev_handle->Get32Bits(0xBF50B004, &ulTmp);
			ulTmp &= 0xFFFFFFE7;
			ulTmp |= 0x10;
			dev_handle->Set32Bits(0xBF50B004, ulTmp);

			//BF50B00C[4]=1  //复位释放
			dev_handle->Get32Bits(0xBF50B00C, &ulTmp);
			ulTmp |= 0x10;
			dev_handle->Set32Bits(0xBF50B00C, ulTmp);

			//BF200010[31:0]=0xC7E101FF//TC1 CTR
			dev_handle->Set32Bits(0xBF200010, 0xC7D801FF);
#endif

			break;

		default:
			printk("%s[%d] -- Unknown chip, demod_type = %d\n", __FUNCTION__, __LINE__, dev_handle->demod_device[2].demod_type);
			break;
	}

	dev_handle->demod_device[2].on_board_settings.chip_mode = 1;

	mt_fe_dmd_tc6930_init(dev_handle, 0);
	mt_fe_dmd_tc6930_init(dev_handle, 1);
	mt_fe_dmd_tc6930_init(dev_handle, 2);

	mt_fe_tn_set_tuner_index_tc6930(dev_handle);

#if 0
	for(i = 0; i < 0x100; i ++)
	{
		dev_handle->dmd_get_reg(dev_handle, gDemodIndex, (U8)i, &tmp);

		printk("\t%d,\t%02x - %02x\n", gDemodIndex, (U8)i, tmp);
	}
#endif

#if 0
	printk("\nDump sym2 system registers\n");

	for(i = 0; i < 0x10; i ++)
	{
		_mt_fe_sym2_t2_dmd_get_reg((U8)i, &tmp);

		printk("\t%02x - %02x\n", (U8)i, tmp);
	}

	printk("\nDump TC6930 demod 0 registers\n");

	for(i = 0; i < 0x100; i ++)
	{
		dev_handle->dmd_get_reg(dev_handle, 0, (U8)i, &tmp);

		printk("\t%02x - %02x\n", (U8)i, tmp);
	}

	printk("\nDump Sym2 DVB-C demod registers\n");

	for(i = 0; i < 0x100; i ++)
	{
		_mt_fe_sym2_c_dmd_get_reg((U8)i, &tmp);

		printk("\t%02x - %02x\n", (U8)i, tmp);
	}


	printk("\n%s[%d] -- attach OK!\n", __FUNCTION__, __LINE__);
#endif

	info->is_attach = 1;

	return MT_SUCCESS;
}

int m88tc6930_detach(frontend_info_s *info)
{
	mt_fe_tc6930_priv_handle p_priv = NULL;
	MT_FE_TC6930_Device_Handle dev_handle = NULL;

	if (info->is_attach)
	{
		p_priv = info->handle;
		dev_handle = p_priv->tc6930_handle;

		//kfree(dev_handle);

		info->is_attach = 0;
	}

	//g_tc6930_priv = NULL;

	//printk("%s[%d] -- detach OK\n", __FUNCTION__, __LINE__);

	return MT_SUCCESS;
}
#endif

