/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
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
#include "mt_mach/clock.h"

#include <mt_unf_frontend.h>
//#include <drv_frontend.h>
#include "drv_frontend_ext.h"
#include "drv_frontend_ioctl.h"
//#include <drv_otp_symphony.h>
//#include "../drv_frontend_priv.h"
#include "mt_fe_def_cs8000_cab.h"
//#include "mt_fe_cab_tn_tc2800.h"
#include "mt_fe_cab_tn_tc3800.h"
#include "mt_fe_common.h"
#include "mt_module_debug.h"

#define CS8K_CAB_X_TAL 28800
#define DEMOD_I2C_ADDR 0x38
/* tuner default i2c address setting */
#define TUNER_TDCC_G051F_I2C_ADDR 0xC0
#define TUNER_I2C_ADDR_TC2800 0xc2

int g_i2c_cs8k_cab = 0;
//MT_FE_CS8000_CAB_TN_SETTINGS dc2800_tuner_ops = {0,};

static int port_m88cs8k_cab_connect(void *handle, mt_unf_fe_connect_para_t *para)
{
    int ret = 0;
    U32 freqKHz = 0;
    U16 symKs = 0;
    U16 qam = 0;
    U8 invert = 0;
    MT_FE_CS8000_CAB_Device_Handle dev_handle = handle;
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

    ret = mt_fe_dmd_cs8k_cab_connect(handle, freqKHz, symKs, qam, invert);
    if (ret < 0)
	return MT_FAILURE;

    return MT_SUCCESS;
}

static int port_m88cs8k_cab_get_status(void *handle, mt_unf_fe_status_t *status)
{
    MT_FE_LOCK_STATE stat = 0;
    mt_fe_dmd_cs8k_cab_get_lock_state(handle, &stat);

    if (stat == MtFeLockState_Unlocked)
	status->lock_status = MT_UNF_FE_SIGNAL_DROPPED;
    else
	status->lock_status = MT_UNF_FE_SIGNAL_LOCKED;

    return MT_SUCCESS;
}

static int port_m88cs8k_cab_get_ber(void *handle, MT_U32 *ber)
{
    U32 errbits = 0;
    U32 total_bits = 0;
    /*
    U32 ber0 = 0;
    U32 ber1 = 0;
    U32 ber2 = 0;
    U32 max = 20;
    */
    int ret = 0;
    ret = mt_fe_dmd_cs8k_cab_get_ber(handle, &errbits, &total_bits);
    if (ret < 0)
	return MT_FAILURE;

#if 0
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
            ber1 = ((errbits - (ber0 * errtot)) * 1000) / errtot;
            break;
        }
    }
#endif

    ber[0] = total_bits;
    ber[1] = errbits;
    ber[2] = 0;

    return MT_SUCCESS;
}

static int port_m88cs8k_cab_get_snr(void *handle, MT_U32 *snr)
{
    U8 _snr = 0;
    int ret = 0;
    ret = mt_fe_dmd_cs8k_cab_get_snr(handle, &_snr);
    if (ret < 0)
	return MT_FAILURE;
    *snr = _snr;
    return MT_SUCCESS;
}

static int port_m88cs8k_cab_get_signal_strength(void *handle, MT_U32 *strength)
{
    U8 _strength = 0;
    int ret = 0;
    ret = mt_fe_dmd_cs8k_cab_get_strength(handle, &_strength);
    if (ret < 0)
	return MT_FAILURE;
    *strength = _strength;
    return MT_SUCCESS;
}

static int port_m88cs8k_cab_get_signal_quality(void *handle, MT_U32 *quality)
{
    *quality = 0;

    return MT_SUCCESS; //-ENOTTY;
}

static int port_m88cs8k_cab_standby(void *handle)
{
    MT_FE_CS8000_CAB_Device_Handle dev_handle = handle;

    if (dev_handle->tuner_settings.tuner_sleep != NULL) {
	dev_handle->tuner_settings.tuner_sleep(handle);
    }

    return MT_SUCCESS;
}

static int port_m88cs8k_cab_wakeup(void *handle)
{
    MT_FE_CS8000_CAB_Device_Handle dev_handle = handle;

    if (dev_handle->tuner_settings.tuner_wakeup != NULL) {
	dev_handle->tuner_settings.tuner_wakeup(handle);
    }

    return MT_SUCCESS;
}

static int port_m88cs8k_cab_get_default_timeout(void *handle, MT_U32 *timeout)
{
    *timeout = 120;
    return MT_SUCCESS;
}

static int port_m88cs8k_cab_set_io(void *handle, MT_BOOL onoff)
{
#if 0
    int ret = 0;
    ret = mt_fe_dmd_cs8k_cab_set_io(handle, onoff);
    if(ret < 0)
        return MT_FAILURE;
#endif
    return MT_SUCCESS;
}

//extern int otp_ioctl(unsigned int cmd, void *arg);
//extern int i2c_gpio_open(u32 bus_clk_khz);
//int m88cs8k_cab_detach(frontend_info_s *info);

int m88cs8k_cab_attach(frontend_info_s *info, mt_unf_fe_attr_t *attr)
{
    MT_FE_CS8000_CAB_Device_Handle dev_handle = NULL;
    U32 dmd_version_no = 0;
    U32 dmd_version_time = 0;
    U32 tn_version_no = 0;
    U32 tn_version_time = 0;
    mt_u32 tn_crystal_hz = 0;
    int ret = 0;
    mt_u8 reg_val = 0;

#if 0
	U32	 temp;
	temp = readl((volatile unsigned int *)0xbf13c010);
	temp &= ~(0xF << 16);
	temp &= ~(0xF << 20);
	temp |= (1 << 16);
	temp |= (1 << 20);
	writel(temp, (volatile unsigned int *)0xbf13c010);

	temp = readl((volatile unsigned int *)0xbf13c008);
	temp &= ~(0xF << 28);
	temp |= (1 << 28);
	writel(temp, (volatile unsigned int *)0xbf13c008);

	temp = readl((volatile unsigned int *)0xbf13c00c);
	temp &= ~(0xF << 4);
	temp |= (2 << 4);
	writel(temp, (volatile unsigned int *)0xbf13c00c);

	writel(0x211, (volatile unsigned int *)0xbf138008);
#endif
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
    info->ops.connect = port_m88cs8k_cab_connect;
    info->ops.get_status = port_m88cs8k_cab_get_status;
    info->ops.get_ber = port_m88cs8k_cab_get_ber;
    info->ops.get_snr = port_m88cs8k_cab_get_snr;
    info->ops.get_signal_strength = port_m88cs8k_cab_get_signal_strength;
    info->ops.get_signal_quality = port_m88cs8k_cab_get_signal_quality;
    info->ops.standby = port_m88cs8k_cab_standby;
    info->ops.wakeup = port_m88cs8k_cab_wakeup;
    info->ops.get_default_timeout = port_m88cs8k_cab_get_default_timeout;
    info->ops.set_io = port_m88cs8k_cab_set_io;
    dev_handle = kzalloc(sizeof(MT_FE_CS8000_CAB_SETTINGS), GFP_KERNEL);
    if (dev_handle == NULL)
	return -ENOMEM;
    info->handle = (void *)dev_handle;

    if (0) //NULL != attr->pstFrontEndConfig)
    {
	/*
		if (0 != p_nim_cfg->x_crystal)
		{
			p_priv->x_crystal = p_nim_cfg->x_crystal;
		}
		else
		{
			p_priv->x_crystal = X_TAL;
		}

		ts_mode = p_nim_cfg->ts_mode;

		if (p_nim_cfg->dem_addr != 0)
		{
			p_priv->dmd_addr = p_nim_cfg->dem_addr;
		}
		else
		{
			p_priv->dmd_addr = DEMOD_I2C_ADDR;
		}

		if (p_nim_cfg->dem_ver == DEM_VER_0)
		{
			p_priv->chip_mode = 2;
		}
		else if (p_nim_cfg->dem_ver == DEM_VER_1)
		{
			p_priv->chip_mode = 0;
		}
		else
		{
			p_priv->chip_mode = 1;
		}


		p_priv->tuner_loopthrough = p_nim_cfg->tuner_loopthrough;
		if(p_nim_cfg->tuner_bandwidth == 0)
		{
			p_priv->tuner_bandwidth = 8;
		}
		else
		{
			p_priv->tuner_bandwidth = p_nim_cfg->tuner_bandwidth;
		}
		if(p_priv->tuner_mode != 0)
		{
			p_priv->tuner_mode = p_nim_cfg->tuner_mode;
		}
		*/
    } else {
	/*
		p_priv->x_crystal = X_TAL;
		p_priv->dmd_addr = DEMOD_I2C_ADDR;
		p_priv->chip_mode = 0;
		p_priv->tuner_loopthrough = 0;
		p_priv->tuner_bandwidth = 8;
		p_priv->tuner_mode = 0;
		*/
    }

    g_i2c_cs8k_cab = attr->tuner_i2c_id[0];
    /* init nim hw */
    //mt_fe_dmd_cs8k_cab_config_default(&p_priv->handle);
    mt_fe_dmd_cs8k_cab_config_default(dev_handle);
    //dev_handle->on_board_settings.i2c_id = attr->i2c_id;
    //dev_handle->demod_i2c_id = attr->enI2cChannel;
    //dev_handle->tuner_i2c_id = attr->enI2cChannel;

    if (attr->demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88CS8000_CAB)
	dev_handle->on_board_settings.chip_mode = 1;
    else if (attr->demod_dev_type == MT_UNF_DEMOD_DEV_TYPE_M88DC2800)
	dev_handle->on_board_settings.chip_mode = 2;

#if 0
	p_priv->handle.demod_dev_addr = p_priv->dmd_addr;
	p_priv->handle.on_board_settings.chip_mode = p_priv->chip_mode;
	p_priv->handle.on_board_settings.xtal_KHz = p_priv->x_crystal;
	p_priv->handle.tuner_settings.tuner_loopthrough = p_priv->tuner_loopthrough;
	p_priv->handle.tuner_settings.tuner_bandwidth = p_priv->tuner_bandwidth;
	p_priv->handle.tuner_settings.tuner_mode = p_priv->tuner_mode;
#else
    dev_handle->demod_dev_addr = attr->demod_addr;

//FIXME:
//symphony4 fpga: chip_mode = 2, DC2800
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    dev_handle->on_board_settings.chip_mode = 2;
#else
    dev_handle->on_board_settings.chip_mode = 1;             //attr->chip_mode;
#endif
    dev_handle->on_board_settings.xtal_KHz = CS8K_CAB_X_TAL; //attr->x_crystal;
    dev_handle->tuner_settings.tuner_dev_addr = attr->tuner_addr;
    dev_handle->tuner_settings.tuner_loopthrough = 0; //attr->tuner_loopthrough;
    dev_handle->tuner_settings.tuner_bandwidth = 8;   //attr->u32TunerAddr;
    dev_handle->tuner_settings.tuner_mode = 0; //attr->tuner_mode;
#endif
    if (attr->tuner_type == MT_UNF_TUNER_TYPE_M88TC2800) {
	mt_fe_dmd_cs8k_cab_select_tuner(dev_handle, TN_MONTAGE_TC2800);
    } else if (attr->tuner_type == MT_UNF_TUNER_TYPE_M88TC3800) {
	mt_fe_dmd_cs8k_cab_select_tuner(dev_handle, TN_MONTAGE_TC3800);
    } else if (attr->tuner_type == MT_UNF_TUNER_TYPE_M88TC6800) {
	mt_fe_dmd_cs8k_cab_select_tuner(dev_handle, TN_MONTAGE_TC6800);
    } else if (attr->tuner_type == MT_UNF_TUNER_TYPE_R836) {
	mt_fe_dmd_cs8k_cab_select_tuner(dev_handle, TN_RAFAEL_R836);
    }
#if 0
    else if (attr->enTunerDevType == MT_UNF_TUNER_DEV_TYPE_TDA18250A)
    {
        mt_fe_dmd_cs8k_cab_select_tuner(dev_handle, TN_NXP_TDA18250A);
    }
    else if (attr->enTunerDevType == MT_UNF_TUNER_DEV_TYPE_MXL_608)
    {
        mt_fe_dmd_cs8k_cab_select_tuner(dev_handle, TN_MXL_608);
    }
#endif
    else {
	kfree((void *)dev_handle);
	return -EINVAL;
    }

    //p_priv->tuner_type = attr->enTunerDevType;
    if (attr->tuner_type == MT_UNF_TUNER_TYPE_M88TC3800_FOR_WASU) {
	dev_handle->on_board_settings.iGainStep = 2;
	dev_handle->on_board_settings.iVppSel = 1;
    } else {
	dev_handle->on_board_settings.iGainStep = 0;
	dev_handle->on_board_settings.iVppSel = 0;
    }

#if 1
    if (attr->output_mode == MT_UNF_FE_OUTPUT_MODE_PARALLEL)
	dev_handle->ts_output_settings.output_mode = MtFeTsOutMode_Parallel;
    else
	dev_handle->ts_output_settings.output_mode = MtFeTsOutMode_Serial;
#endif

    /*
	if (dev_handle->tuner_settings.tuner_self_check != NULL)
	{
		ret = dev_handle->tuner_settings.tuner_self_check(dev_handle);
		if(ret == 0)
		{
			m88cs8k_cab_detach(info);
			return -1;
		}
	}
	*/
    ret = mt_fe_dmd_cs8k_cab_init(dev_handle);
    if (ret != MtFeErr_Ok) {
	if (dev_handle->tuner_settings.tuner_handle)
	    kfree((void *)dev_handle->tuner_settings.tuner_handle);
	kfree((void *)dev_handle);
	return MT_FAILURE;
    }

    ret = dev_handle->dmd_get_reg(dev_handle, 0xe3, &reg_val);

    /* symphony4 fpga: M88DC2800 + TC3800 */
    //if ((ret != MtFeErr_Ok) || (reg_val != 0x80)) {
    if ((ret != MtFeErr_Ok) || (reg_val != 0x80 && reg_val != 0x82)) {
	if (dev_handle->tuner_settings.tuner_handle)
	    kfree((void *)dev_handle->tuner_settings.tuner_handle);
	kfree((void *)dev_handle);
	return MT_FAILURE;
    }

    ret = dev_handle->tn_get_reg(dev_handle, 0x01, &reg_val);
    if (ret != MtFeErr_Ok) {
	if (dev_handle->tuner_settings.tuner_handle)
	    kfree((void *)dev_handle->tuner_settings.tuner_handle);
	kfree((void *)dev_handle);
	return MT_FAILURE;
    }

#if 0
    if(attr->enTunerDevType == MT_UNF_TUNER_DEV_TYPE_M88TC3800_FOR_WASU)
    {
        MT_FE_Tuner_Handle_TC3800 t = NULL;
        t = (MT_FE_Tuner_Handle_TC3800)dev_handle->tuner_settings.tuner_handle;
        t->tuner_gpio_out = 1;
    }
#endif

    symphony_get_clock(HAL_LEDKB, (unsigned long *)&tn_crystal_hz);

//FIXME:
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    if (tn_crystal_hz == 0)
        tn_crystal_hz = 24000000;
#endif

    if (tn_crystal_hz == 0) {
	if (dev_handle->tuner_settings.tuner_handle)
	    kfree((void *)dev_handle->tuner_settings.tuner_handle);
	kfree((void *)dev_handle);
	return MT_FAILURE;
    }
    if (dev_handle->tuner_settings.tuner_set_crystal)
	dev_handle->tuner_settings.tuner_set_crystal(dev_handle, tn_crystal_hz / 1000);

    if (attr->output_mode == MT_UNF_FE_OUTPUT_MODE_PARALLEL) {
	dev_handle->ts_output_settings.output_mode = MtFeTsOutMode_Parallel;
	_mt_fe_dmd_cs8k_cab_set_ts_output(dev_handle, MtFeTsOutMode_Parallel);
//    } else if (attr->output_mode == MtFeTsOutMode_Serial) {
	} else if (attr->output_mode == MT_UNF_FE_OUTPUT_MODE_SERIAL) {
	dev_handle->ts_output_settings.output_mode = MtFeTsOutMode_Serial;
	_mt_fe_dmd_cs8k_cab_set_ts_output(dev_handle, MtFeTsOutMode_Serial);
    } else {
	dev_handle->ts_output_settings.output_mode = MtFeTsOutMode_Common;
	_mt_fe_dmd_cs8k_cab_set_ts_output(dev_handle, MtFeTsOutMode_Common);
    }

    //MT_INFO_FRONTEND("i2c_id %d enDemodDevType %d enTunerDevType %d\n", g_i2c_cs8k_cab, attr->enDemodDevType, attr->enTunerDevType);
    //MT_INFO_FRONTEND("enOutputMode %d u32DemodAddr 0x%02x u32TunerAddr 0x%02x\n", attr->enOutputMode, attr->u32DemodAddr, attr->u32TunerAddr);

    mt_fe_dmd_cs8k_cab_get_driver_version(&dmd_version_no, &dmd_version_time);
    dev_handle->tuner_settings.tuner_get_version(dev_handle, &tn_version_no, &tn_version_time);

    MT_INFO_FRONTEND("\n========Demod Version Info========\n");
    MT_INFO_FRONTEND("    version no: %d\n", dmd_version_no);
    MT_INFO_FRONTEND("    version time: %d\n", dmd_version_time);
    MT_INFO_FRONTEND("========Tuner Version Info========\n");
    MT_INFO_FRONTEND("    version no: %d\n", tn_version_no);
    MT_INFO_FRONTEND("    version time: %d\n", tn_version_time);

    //memcpy(&info->pre_attr, attr, sizeof(mt_unf_fe_attr_t));

    info->is_attach = 1;

    return MT_SUCCESS;
}

int m88cs8k_cab_detach(frontend_info_s *info)
{
    MT_FE_CS8000_CAB_Device_Handle dev_handle = NULL;

    if (info->is_attach) {
	dev_handle = info->handle;
	if (dev_handle->tuner_settings.tuner_handle) {
	    kfree((void *)dev_handle->tuner_settings.tuner_handle);
	}

	kfree((void *)dev_handle);
	info->is_attach = 0;
    }

    return 0;
}

#if 0
void tuner_attach_MxL608(void)
{
    dc2800_tuner_type = TN_MXL_608;
    dc2800_tuner_ops.tuner_init            = mt_fe_cs8k_cab_tn_init_MXL608;
    dc2800_tuner_ops.tuner_set             = mt_fe_cs8k_cab_tn_set_freq_MXL608;
    dc2800_tuner_ops.tuner_get_offset      = NULL;
    dc2800_tuner_ops.tuner_get_strength    = mt_fe_cs8k_cab_tn_get_strength_MXL608;

    dc2800_tuner_ops.tuner_sleep           = NULL;
    dc2800_tuner_ops.tuner_wakeup          = NULL;
    dc2800_tuner_ops.tuner_get_version     = mt_fe_cs8k_cab_tn_get_version_default;

    dc2800_tuner_ops.tuner_self_check      = mt_fe_cs8k_cab_tn_self_check_MxL608;
}

void tuner_attach_MxL203(void)
{
    dc2800_tuner_type = TN_MXL_203;
    dc2800_tuner_ops.tuner_init            = mt_fe_cs8k_cab_tn_init_MxL203;
    dc2800_tuner_ops.tuner_set             = mt_fe_cs8k_cab_tn_set_freq_MxL203;
    dc2800_tuner_ops.tuner_get_offset      = NULL;
    dc2800_tuner_ops.tuner_get_strength    = mt_fe_cs8k_cab_tn_get_strength_MxL203;

    dc2800_tuner_ops.tuner_sleep           = NULL;
    dc2800_tuner_ops.tuner_wakeup          = NULL;
    dc2800_tuner_ops.tuner_get_version     = NULL;

    dc2800_tuner_ops.tuner_self_check      = mt_fe_cs8k_cab_tn_self_check_MxL203;
}

void tuner_attach_TDA18250(void)
{
    dc2800_tuner_type = TN_NXP_TDA18250;
    dc2800_tuner_ops.tuner_init            = mt_fe_cs8k_cab_tn_init_TDA18250;
    dc2800_tuner_ops.tuner_set             = mt_fe_cs8k_cab_tn_set_freq_TDA18250;
    dc2800_tuner_ops.tuner_get_offset      = NULL;
    dc2800_tuner_ops.tuner_get_strength    = mt_fe_cs8k_cab_tn_get_strength_TDA18250;

    dc2800_tuner_ops.tuner_sleep           = NULL;
    dc2800_tuner_ops.tuner_wakeup          = NULL;
    dc2800_tuner_ops.tuner_get_version     = mt_fe_cs8k_cab_tn_get_version_TDA18250;

    dc2800_tuner_ops.tuner_self_check      = mt_fe_cs8k_cab_tn_self_check_TDA18250;

}

void tuner_attach_TDA18250A(void)
{
    dc2800_tuner_type = TN_NXP_TDA18250A;
    dc2800_tuner_ops.tuner_init            = mt_fe_cs8k_cab_tn_init_TDA18250A;
    dc2800_tuner_ops.tuner_set             = mt_fe_cs8k_cab_tn_set_freq_TDA18250A;
    dc2800_tuner_ops.tuner_get_offset      = NULL;
    dc2800_tuner_ops.tuner_get_strength    = mt_fe_cs8k_cab_tn_get_strength_TDA18250A;

    dc2800_tuner_ops.tuner_sleep           = NULL;
    dc2800_tuner_ops.tuner_wakeup          = NULL;
    dc2800_tuner_ops.tuner_get_version     = NULL;

    dc2800_tuner_ops.tuner_self_check      = mt_fe_cs8k_cab_tn_self_check_TDA18250;
}

void tuner_attach_R836(void)
{
   dc2800_tuner_type = TN_RAFAEL_R836;
   dc2800_tuner_ops.tuner_init            = mt_fe_cs8k_cab_tn_init_R836;
   dc2800_tuner_ops.tuner_set             = mt_fe_cs8k_cab_tn_set_freq_R836;
   dc2800_tuner_ops.tuner_get_offset      = NULL;
   dc2800_tuner_ops.tuner_get_strength    = mt_fe_cs8k_cab_tn_get_strength_R836;

   dc2800_tuner_ops.tuner_sleep           = NULL;
   dc2800_tuner_ops.tuner_wakeup          = NULL;
   dc2800_tuner_ops.tuner_get_version     = NULL;

   dc2800_tuner_ops.tuner_self_check      = mt_fe_cs8k_cab_tn_self_check_R836;
}
#endif
