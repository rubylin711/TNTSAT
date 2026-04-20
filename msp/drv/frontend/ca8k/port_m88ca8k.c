/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
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
#include <mt_unf_frontend.h>
//#include <drv_frontend.h>
#include <drv_frontend_ioctl.h>
//#include <drv_otp_symphony.h>
//#include "../drv_frontend_priv.h"
#include "mt_fe_def_ca8k.h"
#include "mt_fe_tn_tc6800.h"
#include "mt_fe_common.h"
#include "mt_fe_i2c.h"
#include <linux/delay.h>
#include "mt_module_debug.h"

#ifdef CONFIG_MT_CHIP_ARIA
#include <mach/aria_io.h>
#else
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#endif

static int port_m88ca8k_connect(void *handle, mt_unf_fe_connect_para_t *para)
{
    int ret = 0;
    U32 freqKHz = 0;
    U16 symKs = 0;
    U16 qam = 0;
    U8 invert = 0;
	//U32 cnt = 0;
	//U32 lock_timeout_ms = 800;
    //MT_FE_LOCK_STATE lock_st = 0;
	U32 value1 = 0, value2 = 0, value3 = 0, value4 = 0;

    MT_FE_CA8K_CAB_Device_Handle dev_handle = handle;
    freqKHz = para->connect_param.cab.freq;
    symKs = para->connect_param.cab.sym_rate / 1000;
    if (freqKHz == 0 || symKs == 0)
	return -1;

    invert = para->connect_param.cab.b_reverse;
    dev_handle->tuner_settings.tuner_bandwidth = para->connect_param.cab.band_width;

    if (dev_handle->tuner_settings.tuner_bandwidth != 8 && dev_handle->tuner_settings.tuner_bandwidth != 6 && dev_handle->tuner_settings.tuner_bandwidth != 7) {
	dev_handle->tuner_settings.tuner_bandwidth = 8;
    }

    switch (para->connect_param.cab.mod_type) {
    case MT_UNF_MOD_TYPE_QAM_16:
	qam = 16;
	break;
    case MT_UNF_MOD_TYPE_QAM_32:
	qam = 32;
	break;
    case MT_UNF_MOD_TYPE_QAM_64:
	qam = 64;
	break;
    case MT_UNF_MOD_TYPE_QAM_128:
	qam = 128;
	break;
    case MT_UNF_MOD_TYPE_QAM_256:
	qam = 256;
	break;
    default:
	qam = 64;
	break;
    }

	MT_INFO_FRONTEND("freqKHz %d symKs %d qam %d invert %d.\n", freqKHz, symKs, qam, invert);

    ret = mt_fe_dmd_ca8k_cab_connect(handle, freqKHz, symKs, qam, invert);
    if (ret < 0)
    {
		MT_ERR_FRONTEND("ret (%d)\n",ret);
		return ret;
    }
#if 0
	for (cnt = 0; cnt < lock_timeout_ms; cnt += 80)
	{
		mt_fe_dmd_ca8k_cab_get_lock_state(handle, &lock_st);

		if (MtFeLockState_Locked == lock_st)
		{
			MT_INFO_FRONTEND("[%s] line:%d lock_status = %d.\n", __func__, __LINE__, lock_st);
			return 0;
		}
		mdelay(100);
	}
#endif

	dev_handle->Get32Bits(0xA4, &value1);//0xFFAF00A4
	dev_handle->Get32Bits(0xA0, &value2);//0xFFAF00A4
	dev_handle->Get32Bits(0xB8, &value3);//0xFFAF00A4
	dev_handle->Get32Bits(0xB4, &value4);//0xFFAF00A4

	MT_INFO_FRONTEND("value1 = 0x%08x value2= 0x%08x, value3 = 0x%08x, value4 = 0x%08x.\n", value1, value2, value3, value4);

    return 0;
}

static int port_m88ca8k_get_status(void *handle, mt_unf_fe_status_t *status)
{
    MT_FE_LOCK_STATE stat = 0;
    mt_fe_dmd_ca8k_cab_get_lock_state(handle, &stat);

    if (stat != MtFeLockState_Locked)
	status->lock_status = MT_UNF_FE_SIGNAL_DROPPED;
    else
	status->lock_status = MT_UNF_FE_SIGNAL_LOCKED;

	//MT_INFO_FRONTEND("port_m88ca8k_get_status (%d %d).\n", stat, status->lock_status);

	return 0;
}

static int port_m88ca8k_get_ber(void *handle, MT_U32 *ber)
{
    U32 errbits = 0;
    U32 errtot = 0;
    U32 ber0 = 0;
    U32 ber1 = 0;
    U32 ber2 = 0;
    U32 max = 20;
    int ret = 0;

    ret = mt_fe_dmd_ca8k_cab_get_ber(handle, &errbits, &errtot);
    if (ret < 0)
	return ret;
    while (max--) {
	if ((errbits / errtot) == 0) {
	    errbits *= 10;
	    ber2++;
	} else {
	    ber0 = errbits / errtot;
	    ber1 = ((errbits - (ber0 * errtot)) * 1000) / errtot;
	    break;
	}
    }
    ber[0] = ber0;
    ber[1] = ber1;
    ber[2] = ber2;

    return 0;
}

static int port_m88ca8k_get_snr(void *handle, MT_U32 *snr)
{
    U8 _snr = 0;
    int ret = 0;
    ret = mt_fe_dmd_ca8k_cab_get_snr(handle, &_snr);
    if (ret < 0)
	return ret;
    *snr = _snr;
    return 0;
}

static int port_m88ca8k_get_signal_strength(void *handle, MT_U32 *strength)
{
    U8 _strength = 0;
    int ret = 0;
    ret = mt_fe_dmd_ca8k_cab_get_strength(handle, &_strength);
    if (ret < 0)
	return ret;
    *strength = _strength;
    return 0;
}

static int port_m88ca8k_get_signal_quality(void *handle, MT_U32 *quality)
{
    return -ENOTTY;
}

static int port_m88ca8k_standby(void *handle)
{
    MT_FE_CA8K_CAB_Device_Handle dev_handle = handle;

    if (dev_handle->tuner_settings.tuner_sleep != NULL) {
	dev_handle->tuner_settings.tuner_sleep(handle);
    }

    return 0;
}
static int port_m88ca8k_wakeup(void *handle)
{
    MT_FE_CA8K_CAB_Device_Handle dev_handle = handle;

    if (dev_handle->tuner_settings.tuner_wakeup != NULL) {
	dev_handle->tuner_settings.tuner_wakeup(handle);
    }

    return 0;
}
static int port_m88ca8k_get_default_timeout(void *handle, MT_U32 *timeout)
{
    *timeout = 120;
    return 0;
}

static int port_m88ca8k_set_io(void *handle, MT_BOOL onoff)
{
#if 0
    int ret = 0;
    ret = mt_fe_dmd_ca8k_cab_set_io(handle, onoff);
    if(ret < 0)
        return ret;
#endif
    return 0;
}

//extern int otp_ioctl(unsigned int cmd, void *arg);
//extern int i2c_gpio_open(u32 bus_clk_khz);
static void m88ca8k_set_pinmux(void)
{
    u32 data = 0;
    u32 sysctrl_base;
    sysctrl_base = mt_get_sys_ctrl_base();
    //MT_INFO_FRONTEND("\n pinmux config.......\n");
    //mdelay(5000);
	data = readl((volatile int *)(sysctrl_base + 0x2c));
    data &= ~(1 << 23);
    data |= (1 << 27);
    writel(data, (volatile int *)(sysctrl_base + 0x2c));
}

int m88ca8k_attach(frontend_info_s *info, mt_unf_fe_attr_t *attr)
{
    MT_FE_CA8K_CAB_Device_Handle dev_handle = NULL;
    U32 version_no = 0;
    U32 version_time = 0;
    U32 result = 0;
	U8 tuner_data = 0;
	U8 demod_tmp1 = 0;
	U8 demod_tmp2 = 0;

	int ret = 0;
    MT_FE_Tuner_Handle_TC6800 tuner_handle = NULL;

	//MT_INFO_FRONTEND("m88ca8k_attach line:%d.\n", __LINE__);

#if 0
    otp_ioctl(CMD_GET_DEMO_EN, &result);
    if(result == 0)
        return -EIO;
    if(attr->enTunerDevType == MT_UNF_TUNER_DEV_TYPE_M88TC3800_FOR_WASU ||
        attr->enTunerDevType == MT_UNF_TUNER_DEV_TYPE_R836 ||
        attr->enTunerDevType == MT_UNF_TUNER_DEV_TYPE_TDA18250A ||
        attr->enTunerDevType == MT_UNF_TUNER_DEV_TYPE_MXL_608)
    {
        i2c_gpio_open(300);
    }
    else
    {
        i2c_gpio_open(600);
    }
#endif

    info->ops.connect = port_m88ca8k_connect;
    info->ops.get_status = port_m88ca8k_get_status;
    info->ops.get_ber = port_m88ca8k_get_ber;
    info->ops.get_snr = port_m88ca8k_get_snr;
    info->ops.get_signal_strength = port_m88ca8k_get_signal_strength;
    info->ops.get_signal_quality = port_m88ca8k_get_signal_quality;
    info->ops.standby = port_m88ca8k_standby;
    info->ops.wakeup = port_m88ca8k_wakeup;
    info->ops.get_default_timeout = port_m88ca8k_get_default_timeout;
    info->ops.set_io = port_m88ca8k_set_io;

    dev_handle = kzalloc(sizeof(MT_FE_CA8K_CAB_SETTINGS), GFP_KERNEL);
    if (dev_handle == NULL)
	return -ENOMEM;
    info->handle = (void *)dev_handle;

	m88ca8k_set_pinmux();
	//MT_INFO_FRONTEND("m88ca8k_attach line:%d.\n", __LINE__);

    mt_fe_dmd_ca8k_cab_config_default(dev_handle);

    //dev_handle->on_board_settings.i2c_id = attr->i2c_id;
    dev_handle->demod_i2c_id = 0;
    dev_handle->tuner_i2c_id = 0;

    if (attr->demod_dev_type == MT_UNF_DEMOD_TYPE_M88DVBC)
	dev_handle->on_board_settings.chip_mode = 1;
    else if (attr->demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88DC2800)
	dev_handle->on_board_settings.chip_mode = 2;

    dev_handle->demod_dev_addr = attr->demod_addr;
    dev_handle->tuner_settings.tuner_dev_addr = attr->tuner_addr;

    //dev_handle->on_board_settings.chip_mode = p_priv->chip_mode;
    //dev_handle->on_board_settings.xtal_KHz = p_priv->x_crystal;
    //dev_handle->tuner_settings.tuner_loopthrough = 0;
    //dev_handle->tuner_settings.tuner_bandwidth = p_priv->tuner_bandwidth;
    //dev_handle->tuner_settings.tuner_mode = p_priv->tuner_mode;


#if 0
    if (attr->tuner_type == MT_UNF_TUNER_TYPE_M88TC6800) {
	mt_fe_dmd_ca8k_cab_select_tuner(dev_handle, TN_MONTAGE_TC6800);
    } else if (attr->tuner_type == MT_UNF_TUNER_TYPE_R836) {
	mt_fe_dmd_ca8k_cab_select_tuner(dev_handle, TN_RAFAEL_R836);
    } else if (attr->tuner_type == MT_UNF_TUNER_TYPE_TDA18250A) {
	mt_fe_dmd_ca8k_cab_select_tuner(dev_handle, TN_NXP_TDA18250A);
    } else if (attr->tuner_type == MT_UNF_TUNER_TYPE_MXL_608) {
	mt_fe_dmd_ca8k_cab_select_tuner(dev_handle, TN_MXL_608);
    } else
	return -EINVAL;
#endif

	mt_fe_dmd_ca8k_cab_select_tuner(dev_handle, TN_MONTAGE_TC6800);

	MT_INFO_FRONTEND("m88ca8k_attach tuner_type %d out_mode %d.\n", attr->tuner_type, attr->output_mode);

#if 0
    if (attr->tuner_type == MT_UNF_TUNER_TYPE_M88TC3800_FOR_WASU) {
	dev_handle->on_board_settings.iGainStep = 2;
	dev_handle->on_board_settings.iVppSel = 1;
    } else {
	dev_handle->on_board_settings.iGainStep = 0;
	dev_handle->on_board_settings.iVppSel = 0;
    }
#endif

    if (attr->output_mode == MT_UNF_FE_OUTPUT_MODE_PARALLEL)
    {
		dev_handle->ts_output_settings.output_mode = MtFeTsOutMode_Parallel;
 		_mt_fe_dmd_ca8k_cab_set_ts_output(dev_handle, MtFeTsOutMode_Parallel);
 	}
    else if (attr->output_mode == MT_UNF_FE_OUTPUT_MODE_SERIAL)
    {
		dev_handle->ts_output_settings.output_mode = MtFeTsOutMode_Serial;
 		_mt_fe_dmd_ca8k_cab_set_ts_output(dev_handle, MtFeTsOutMode_Serial);
 	}
	else
	{
		dev_handle->ts_output_settings.output_mode = MtFeTsOutMode_Common;
        _mt_fe_dmd_ca8k_cab_set_ts_output(dev_handle, MtFeTsOutMode_Common);
	}

    ret = mt_fe_dmd_ca8k_cab_init(dev_handle);
    if (ret < 0)
    {
		return ret;
    }

#if 0
    if(attr->tuner_type == MT_UNF_TUNER_TYPE_M88TC3800_FOR_WASU)
    {
        MT_FE_Tuner_Handle_TC3800 t = NULL;
        t = (MT_FE_Tuner_Handle_TC3800)dev_handle->tuner_settings.tuner_handle;
        t->tuner_gpio_out = 1;
    }
#endif

    memcpy(&info->pre_attr, attr, sizeof(mt_unf_fe_attr_t));
    info->is_attach = 1;

	tuner_handle = (MT_FE_Tuner_Handle_TC6800)(dev_handle->tuner_settings.tuner_handle);
	_mt_fe_tn_get_reg_tc6800(tuner_handle, 0x01, &tuner_data);
    MT_INFO_FRONTEND("-------------tc6800------tuner reg0x01 = 0x%02x----------------\n", tuner_data);

	_mt_fe_dmd_get_reg((void *)dev_handle, 0x00, &demod_tmp1);
	_mt_fe_dmd_get_reg((void *)dev_handle, 0x01, &demod_tmp2);
    MT_INFO_FRONTEND("------------demod reg0x00 = 0x%02x reg0x01 = 0x%02x----------\n", demod_tmp1, demod_tmp2);

    MT_INFO_FRONTEND("-------------------------------------------------------------\n");
    MT_INFO_FRONTEND("        Demodulator and Tuner Version Information\n");
    mt_fe_dmd_ca8k_cab_get_driver_version(&version_no, &version_time);
    MT_INFO_FRONTEND("    Demodulator: M88CA8K  Version:%d  Time:%d\n",
           version_no, version_time);
    mt_fe_dmd_ca8k_cab_get_tuner_version(dev_handle, &version_no, &version_time);

    if (dev_handle->tuner_settings.tuner_type == TN_MONTAGE_TC2800) {
	MT_INFO_FRONTEND("    Tuner: M88TC2800    Version:%d  Time:%d\n",
	       version_no, version_time);
    } else if (dev_handle->tuner_settings.tuner_type == TN_MONTAGE_TC3800) {
	MT_INFO_FRONTEND("    Tuner: M88TC3800    Version:%d  Time:%d\n",
	       version_no, version_time);
    } else if (dev_handle->tuner_settings.tuner_type == TN_MONTAGE_TC6800) {
	MT_INFO_FRONTEND("    Tuner: M88TC6800    Version:%d  Time:%d\n",
	       version_no, version_time);
    } else if (dev_handle->tuner_settings.tuner_type == TN_RAFAEL_R836) {
	MT_INFO_FRONTEND("    Tuner: RAFAEL R836    Version:%d  Time:%d\n",
	       version_no, version_time);
    } else if (dev_handle->tuner_settings.tuner_type == TN_NXP_TDA18250A) {
	MT_INFO_FRONTEND("    Tuner: TN_NXP_TDA18250A    Version:%d  Time:%d\n",
	       version_no, version_time);
    } else if (dev_handle->tuner_settings.tuner_type == TN_MXL_608) {
	MT_INFO_FRONTEND("    Tuner: TN_MXL_608    Version:%d  Time:%d\n",
	       version_no, version_time);
    } else {
	MT_INFO_FRONTEND("    Tuner: Unknown  Version:%d  Time:%d\n",
	       version_no, version_time);
    }
    MT_INFO_FRONTEND("-------------------------------------------------------------\n");

    return 0;
}

int m88ca8k_detach(frontend_info_s *info)
{
    MT_FE_CA8K_CAB_Device_Handle dev_handle = NULL;

    if (info->is_attach) {
	dev_handle = info->handle;
	if (0) //info->pre_attr.tuner_type == MT_UNF_TUNER_TYPE_M88TC6800)
	{
	    kfree((void *)dev_handle->tuner_settings.tuner_handle);
	}

	kfree((void *)dev_handle);
	info->is_attach = 0;
    }

    return 0;
}
