/*****************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd                                    */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2016 Montage Technology Group Limited. All Rights Reserved. */
/*****************************************************************************/
/*
 * Filename:        mt_fe_dmd_cs8000_cab_tuner.c
 *
 * Description:     Interface of tuner.
 *
 *****************************************************************************/
/*****************************************************************************/
/*     Version:     1.03.05                                                  */
/*********************Supported Tuner List************************************/
/*  Vendor  Name    Model   Version Date    Time    Author      Comments     */
/*  Montage TC2800  -       1.02.03 141117  10:00   YZ.Huang    Update       */
/*  Montage TC3800  -       1.03.03 150922  16:00   YZ.Huang    Update       */
/*  NXP     TDA     18250   1.02.07 150507  11:00   YZ.Huang    Create       */
/*  NXP     TDA     18250A  1.02.07 150507  11:00   YZ.Huang    Create       */
/*  MxL     MXL     203     1.02.09 150731  17:00   YZ.Huang    Create       */
/*  MxL     MxL     608     1.02.09 150728  17:00   YZ.Huang    Create       */
/*	Montage TC6800  -       1.03.05 160620  15:00   YZ.Huang    Update       */
/*  Rafael  R836    -       1.02.10 151019  15:00   YZ.Huang    Create       */
/*  All     Default Tuner   1.02.07 150507  11:00   YZ.Huang    Update       */
/*****************************************************************************/

#include <linux/slab.h>

//#include <string.h>
#include "mt_type.h"
//#include "mtos_mem.h"
#include "mt_fe_i2c.h"

//#include "mt_fe_cab_tn_tc2800.h"
//#include "mt_fe_cab_tn_TDA18250.h"
//#include "mt_fe_cab_tn_tc3800.h"
//#include "mt_fe_cab_tn_MxL608.h"
//#include "mt_fe_cab_tn_MxL203.h"
#include "mt_fe_tn_tc6800.h"
//#include "mt_fe_cab_tn_R836.h"

static MT_FE_CA8K_CAB_Device_Handle demod_handle;

MT_FE_RET mt_fe_cs8k_cab_tn_get_version_default(void *dev_handle, U32 *version_no, U32 *version_time)
{
    *version_no = 10209;      // for Can tuner, use version number of this file
    *version_time = 15073117; // for Can tuner, use version time of this file

    return MtFeErr_Ok;
}

#if 0
/*  Montage TC2800  - 1.00.00 120209  15:00   YZ.Huang    Create              */
/*                    1.00.01 120312  13:00   YZ.Huang    Update              */
/*                    1.00.02 120412  13:00   YZ.Huang    Update  V3.00.08    */
/*                    1.00.06 120524  11:00   YZ.Huang    Update  V3.00.12    */
/*                    1.00.07 120605  18:00   YZ.Huang    Update  V3.00.14    */
/*                    1.00.08 120606  16:00   YZ.Huang    Update  V3.00.15    */
/*                    1.00.09 120629  15:00   YZ.Huang    Update  V3.00.17    */
/*                    1.00.10 120704  11:00   YZ.Huang    Update  V3.00.18    */
/*                    1.00.11 120713  11:00   YZ.Huang    Update  V3.00.19    */
/*                    1.00.12 120725  15:00   YZ.Huang    Update  V3.00.20    */
/*                    1.00.13 121016  10:00   YZ.Huang    Update  V3.00.22    */
/*                    1.00.15 130108  14:00   YZ.Huang    Update  V3.00.23    */
/*                    1.00.16 130314  14:00   YZ.Huang    Update  V3.00.24    */
/*                    1.00.17 130520  18:00   YZ.Huang    Update  V3.00.26    */
/*                    1.00.18 130614  11:00   YZ.Huang    Update  V3.00.27    */
/*                    1.00.19 130709  16:00   YZ.Huang    Update  V3.00.28    */
/*                    1.00.20 130724  18:00   YZ.Huang    Update  V3.00.29    */
/*                    1.00.21 140311  19:00   YZ.Huang    Update  V3.00.30    */
/*                    1.02.03 141117  10:00   YZ.Huang    Update  V3.00.31    */
MT_FE_TN_TC2800_SETTINGS tc2800_config;


MT_FE_RET mt_fe_cs8k_cab_tn_init_tc2800(void *dev_handle)
{
	MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;
    MT_FE_TN_TC2800_Handle tn_handle = NULL;

    handle->tuner_settings.tuner_handle = (U32)mtos_malloc(sizeof(MT_FE_TN_TC2800_SETTINGS));
    memset((void *)handle->tuner_settings.tuner_handle, 0, sizeof(MT_FE_TN_TC2800_SETTINGS));

    tn_handle = (MT_FE_TN_TC2800_Handle)handle->tuner_settings.tuner_handle;
    mt_fe_tn_init_tc2800(handle);
    tn_handle->tuner_dev_addr = handle->tuner_settings.tuner_dev_addr;
    handle->tuner_settings.tuner_init_OK = TRUE;

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_cs8k_cab_tn_set_freq_tc2800(void *dev_handle, U32 freq_KHz, U16 symbol_rate_KSs)
{
	MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;

	if(!(handle->tuner_settings.tuner_init_OK))
	{
		return MtFeErr_Uninit;
	}

	handle->channel_param.freq_KHz	 = freq_KHz;
	handle->channel_param.sym_KSs	 = symbol_rate_KSs;

	mt_fe_tn_set_freq_tc2800(handle, freq_KHz);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_cs8k_cab_tn_get_offset_tc2800(void *dev_handle, S32 *freq_offset_KHz)
{
	*freq_offset_KHz = 0;

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_cs8k_cab_tn_get_strength_tc2800(void *dev_handle, U32 *p_gain, U32 *p_strength)
{
	MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;

	*p_gain = 0;

	*p_strength = (U32)(mt_fe_tn_get_signal_strength_tc2800(handle) + 107.0);		// dBm + 107 ->dBuV

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_cs8k_cab_tn_sleep_tc2800(void *dev_handle)
{
	MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;

	mt_fe_tn_sleep_tc2800(handle);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_cs8k_cab_tn_wakeup_tc2800(void *dev_handle)
{
	MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;

	mt_fe_tn_wakeup_tc2800(handle);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_cs8k_cab_tn_get_version_tc2800(void *dev_handle, U32 *version_no, U32 *version_time)
{
	MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;
	MT_FE_TN_TC2800_Handle tuner_handle = (MT_FE_TN_TC2800_Handle)(handle->tuner_settings.tuner_handle);

	*version_no = 0;
	*version_time = 0;
	if(!(handle->tuner_settings.tuner_init_OK))
	{
		return MtFeErr_Uninit;
	}

	*version_no = tuner_handle->tuner_version;
	*version_time = tuner_handle->tuner_time;

	return MtFeErr_Ok;
}

S32 mt_fe_cs8k_cab_tn_self_check_tc2800(void *dev_handle)
{
	S32 ret = 0;

//	MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;

	ret = mt_fe_tn_self_check_tc2800(dev_handle);

	return ret;
}
#endif

#if 0
/*  NXP		TDA     18250   1.02.07 150507  11:00   YZ.Huang    Create        */
MT_FE_RET mt_fe_cs8k_cab_tn_init_TDA18250(void *dev_handle)
{
	MT_FE_RET ret;
	MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;

	ret = mt_fe_tn_init_TDA18250(handle);
    handle->tuner_settings.tuner_type = TN_NXP_TDA18250;
	handle->tuner_settings.tuner_init_OK = (ret == MtFeErr_Ok) ? TRUE : FALSE;

	return ret;
}

MT_FE_RET mt_fe_cs8k_cab_tn_set_freq_TDA18250(void *dev_handle, U32 freq_KHz, U16 symbol_rate_KSs)
{
	return mt_fe_tn_set_freq_TDA18250(dev_handle, freq_KHz, symbol_rate_KSs);
}

MT_FE_RET mt_fe_cs8k_cab_tn_get_strength_TDA18250(void *dev_handle, U32 *p_gain, U32 *p_strength)
{
	return mt_fe_tn_get_signal_strength_TDA18250(dev_handle, p_gain, p_strength);
}

MT_FE_RET mt_fe_cs8k_cab_tn_get_version_TDA18250(void *dev_handle, U32 *version_no, U32 *version_time)
{
	*version_no = 30018;
	*version_time = 11061510;

	return MtFeErr_Ok;
}

S32 mt_fe_cs8k_cab_tn_self_check_TDA18250(void *dev_handle)
{
	S32 ret = 0;

	//MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;

	ret = mt_fe_tn_self_check_TDA18250(dev_handle);

	return ret;
}


/*  NXP		TDA     18250A  1.02.07 150507  11:00   YZ.Huang    Create        */
MT_FE_RET mt_fe_cs8k_cab_tn_init_TDA18250A(void *dev_handle)
{
	MT_FE_RET ret;
	MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;

	ret = mt_fe_tn_init_TDA18250A(handle);
	handle->tuner_settings.tuner_init_OK = (ret == MtFeErr_Ok) ? TRUE : FALSE;

	return ret;
}

MT_FE_RET mt_fe_cs8k_cab_tn_set_freq_TDA18250A(void *dev_handle, U32 freq_KHz, U16 symbol_rate_KSs)
{
	return mt_fe_tn_set_freq_TDA18250A(dev_handle, freq_KHz, symbol_rate_KSs);
}

MT_FE_RET mt_fe_cs8k_cab_tn_get_strength_TDA18250A(void *dev_handle, U32 *p_gain, U32 *p_strength)
{
	return mt_fe_tn_get_signal_strength_TDA18250A(dev_handle, p_gain, p_strength);
}
#endif

#if 0
/*  MxL     MXL     203     1.02.09 150731  17:00   YZ.Huang    Create       */
extern MT_FE_RET mt_fe_tn_init_MxL203(void *dev_handle);
extern MT_FE_RET mt_fe_tn_set_freq_MxL203(void *dev_handle, U32 freq_KHz, U16 symbol_rate_KSs);
extern MT_FE_RET mt_fe_tn_get_signal_strength_MxL203(void *dev_handle, U32 *p_gain, U32
*p_strength);
extern S32 mt_fe_tn_self_check_MxL203(MT_FE_CA8K_CAB_Device_Handle dev_handle);
MT_FE_RET mt_fe_cs8k_cab_tn_init_MxL203(void *dev_handle)
{
	MT_FE_RET ret;
	MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;

	ret = mt_fe_tn_init_MxL203(handle);
	handle->tuner_settings.tuner_init_OK = (ret == MtFeErr_Ok) ? TRUE : FALSE;

	return ret;
}

MT_FE_RET mt_fe_cs8k_cab_tn_set_freq_MxL203(void *dev_handle, U32 freq_KHz, U16 symbol_rate_KSs)
{
	return mt_fe_tn_set_freq_MxL203(dev_handle, freq_KHz, symbol_rate_KSs);
}

MT_FE_RET mt_fe_cs8k_cab_tn_get_strength_MxL203(void *dev_handle, U32 *p_gain, U32 *p_strength)
{
	return mt_fe_tn_get_signal_strength_MxL203(dev_handle, p_gain, p_strength);
}

S32 mt_fe_cs8k_cab_tn_self_check_MxL203(void *dev_handle)
{
	S32 ret = 0;

//	MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;

	ret = mt_fe_tn_self_check_MxL203(dev_handle);

	return ret;
}

/*  MxL     MxL     608     1.02.09 150728  17:00   YZ.Huang    Create        */
MT_FE_RET mt_fe_cs8k_cab_tn_init_MXL608(void *dev_handle)
{
	MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;

	demod_handle = handle;

	handle->tuner_settings.tuner_type = TN_MXL_608;

	mt_fe_tn_init_MxL608(handle);

	handle->tuner_settings.tuner_init_OK = TRUE;

	return MtFeErr_Ok;
}


MT_FE_RET mt_fe_cs8k_cab_tn_set_freq_MXL608(void *dev_handle, U32 freq_KHz, U16 symbol_rate_KSs)
{
	MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;

	MT_FE_RET ret = MtFeErr_Ok;

	ret = mt_fe_tn_set_freq_MxL608(handle, freq_KHz, symbol_rate_KSs);

	return ret;
}

MT_FE_RET mt_fe_cs8k_cab_tn_get_strength_MXL608(void *dev_handle, U32 *p_gain, U32 *p_strength)
{
	return mt_fe_tn_get_signal_strength_MxL608(dev_handle, p_gain, p_strength);
}

S32 mt_fe_cs8k_cab_tn_self_check_MxL608(void *dev_handle)
{
	S32 ret = 0;

	//MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;

	ret = mt_fe_tn_self_check_MxL608(dev_handle);

	return ret;
}
#endif

#if 0
/*  Montage TC3800  - 0.01.04 140414  14:00   YZ.Huang    Create              */
/*                    0.01.04 140414  14:00   YZ.Huang    Update              */
/*                    0.01.05 140723  10:00   YZ.Huang    Update              */
/*                    0.01.06 140827  10:00   YZ.Huang    Update              */
/*                    0.01.07 141117  10:00   YZ.Huang    Update              */
/*                    0.01.08 141211  18:00   YZ.Huang    Update              */
/*                    0.01.09 141223  15:00   YZ.Huang    Update              */
/*                    0.01.14 150325  11:00   YZ.Huang    Update              */
/*                    0.01.24 150922  14:00   YZ.Huang    Update              */
/*                    0.01.25 151010  10:00   YZ.Huang    Update              */
/*                    0.01.26 160308  16:00   YZ.Huang    Update              */
MT_FE_TN_DEVICE_SETTINGS_TC3800 tc3800_config;


MT_FE_RET mt_fe_cs8k_cab_tn_init_tc3800(void *dev_handle)
{
	MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;
    MT_FE_Tuner_Handle_TC3800 tuner_handle = NULL;

        handle->tuner_settings.tuner_handle = (U32)mtos_malloc(sizeof(MT_FE_TN_DEVICE_SETTINGS_TC3800));
        memset((void *)handle->tuner_settings.tuner_handle, 0, sizeof(MT_FE_TN_DEVICE_SETTINGS_TC3800));

	tuner_handle = (MT_FE_Tuner_Handle_TC3800)(handle->tuner_settings.tuner_handle);
        tuner_handle->tuner_dev_addr = handle->tuner_settings.tuner_dev_addr;
        tuner_handle->tuner_mode = handle->tuner_settings.tuner_loopthrough;
        tuner_handle->tuner_bandwidth = handle->tuner_settings.tuner_bandwidth;
        tuner_handle->p_private = handle;
        mt_fe_tn_init_tc3800(tuner_handle);
        handle->tuner_settings.tuner_init_OK = TRUE;

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_cs8k_cab_tn_set_freq_tc3800(void *dev_handle, U32 freq_KHz, U16 symbol_rate_KSs)
{
	MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;
	MT_FE_Tuner_Handle_TC3800 tuner_handle = (MT_FE_Tuner_Handle_TC3800)(handle->tuner_settings.tuner_handle);

	if(!(handle->tuner_settings.tuner_init_OK))
	{
		return MtFeErr_Uninit;
	}

	handle->channel_param.freq_KHz	 = freq_KHz;
	handle->channel_param.sym_KSs	 = symbol_rate_KSs;
         tuner_handle->tuner_bandwidth = handle->tuner_settings.tuner_bandwidth;
	mt_fe_tn_set_freq_tc3800(tuner_handle, freq_KHz);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_cs8k_cab_tn_get_offset_tc3800(void *dev_handle, S32 *freq_offset_KHz)
{
	*freq_offset_KHz = 0;

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_cs8k_cab_tn_get_strength_tc3800(void *dev_handle, U32 *p_gain, U32 *p_strength)
{
	MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;
	MT_FE_Tuner_Handle_TC3800 tuner_handle = (MT_FE_Tuner_Handle_TC3800)(handle->tuner_settings.tuner_handle);

	*p_gain = 0;

	*p_strength = (U32)(mt_fe_tn_get_signal_strength_tc3800(tuner_handle) + 107.0);		// dBm + 107 ->dBuV

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_cs8k_cab_tn_sleep_tc3800(void *dev_handle)
{
	MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;
	MT_FE_Tuner_Handle_TC3800 tuner_handle = (MT_FE_Tuner_Handle_TC3800)(handle->tuner_settings.tuner_handle);

	mt_fe_tn_sleep_tc3800(tuner_handle);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_cs8k_cab_tn_wakeup_tc3800(void *dev_handle)
{
	MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;
	MT_FE_Tuner_Handle_TC3800 tuner_handle = (MT_FE_Tuner_Handle_TC3800)(handle->tuner_settings.tuner_handle);

	mt_fe_tn_wakeup_tc3800(tuner_handle);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_cs8k_cab_tn_get_version_tc3800(void *dev_handle, U32 *version_no, U32 *version_time)
{
	MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;
	MT_FE_Tuner_Handle_TC3800 tuner_handle = (MT_FE_Tuner_Handle_TC3800)(handle->tuner_settings.tuner_handle);

	*version_no = 0;
	*version_time = 0;
	if(!(handle->tuner_settings.tuner_init_OK))
	{
		return MtFeErr_Uninit;
	}

	*version_no = tuner_handle->tuner_version;
	*version_time = tuner_handle->tuner_time;

	return MtFeErr_Ok;
}

S32 mt_fe_cs8k_cab_tn_self_check_tc3800(void *dev_handle)
{
	S32 ret = 0;

	MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;
	MT_FE_Tuner_Handle_TC3800 tuner_handle = (MT_FE_Tuner_Handle_TC3800)(handle->tuner_settings.tuner_handle);

	if((tuner_handle == NULL) || (!tuner_handle->tuner_init_OK))
	{
		mt_fe_cs8k_cab_tn_init_tc3800(dev_handle);
		tuner_handle = (MT_FE_Tuner_Handle_TC3800)(handle->tuner_settings.tuner_handle);
	}

	ret = mt_fe_tn_self_check_tc3800(tuner_handle);

	return ret;
}

MT_FE_CS8000_CAB_SUPPORTED_TUNER mt_fe_cs8k_cab_check_tuner_type(void *dev_handle)
{
	MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;
	MT_FE_CS8000_CAB_SUPPORTED_TUNER tuner_type = handle->tuner_settings.tuner_type;

	MT_FE_RET ret;

	if((handle->tuner_settings.tuner_type == TN_MONTAGE_TC2800) || (handle->tuner_settings.tuner_type == TN_MONTAGE_TC3800))
	{
		if(handle->tn_get_reg != NULL)
		{
			U8 Reg01H = 0;

			ret = handle->tn_get_reg(handle, 0x01, &Reg01H);
			if(ret != MtFeErr_Ok)
			{
				return tuner_type;
			}

			if((Reg01H & 0xF0) == 0xB0)
			{
				tuner_type = TN_MONTAGE_TC3800;
			}
			else if((Reg01H == 0x0D) || (Reg01H == 0x8E))
			{
				tuner_type = TN_MONTAGE_TC2800;
			}
			else
			{
				return tuner_type;
			}
		}
	}

	return tuner_type;
}
#endif

/*	Montage TC6800  -       0.01.01 160223  15:00   YZ.Huang    Create       */
/*	                -       0.01.06 160413  18:00   YZ.Huang    Modify       */
/*	                -       0.01.10 160429  15:00   YZ.Huang    Modify       */
/*	                -       0.01.12 160620  15:00   YZ.Huang    Modify       */
MT_FE_TN_DEVICE_SETTINGS_TC6800 tc6800_config;

MT_FE_RET mt_fe_tn_init_tc6800_ca8k(void *dev_handle)
{
    MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;
    MT_FE_Tuner_Handle_TC6800 tuner_handle = NULL;

    handle->tuner_settings.tuner_handle = (U32)kzalloc(sizeof(MT_FE_TN_DEVICE_SETTINGS_TC6800), GFP_KERNEL);
    memset((void *)handle->tuner_settings.tuner_handle, 0, sizeof(MT_FE_TN_DEVICE_SETTINGS_TC6800));
    tuner_handle = (MT_FE_Tuner_Handle_TC6800)(handle->tuner_settings.tuner_handle);
    tuner_handle->tuner_dev_addr = handle->tuner_settings.tuner_dev_addr;
    tuner_handle->tuner_mode = handle->tuner_settings.tuner_loopthrough;
    tuner_handle->tuner_bandwidth = handle->tuner_settings.tuner_bandwidth * 1000;
    tuner_handle->p_private = handle;

    mt_fe_tn_init_tc6800(tuner_handle);

    handle->tuner_settings.tuner_init_OK = TRUE;

    return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_set_freq_tc6800_ca8k(void *dev_handle, U32 freq_KHz, U16 symbol_rate_KSs)
{
    MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;
    MT_FE_Tuner_Handle_TC6800 tuner_handle = (MT_FE_Tuner_Handle_TC6800)(handle->tuner_settings.tuner_handle);

    if (!(handle->tuner_settings.tuner_init_OK)) {
	return MtFeErr_Uninit;
    }

    handle->channel_param.freq_KHz = freq_KHz;
    handle->channel_param.sym_KSs = symbol_rate_KSs;

    mt_fe_tn_set_freq_tc6800(tuner_handle, freq_KHz);

    return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_get_offset_tc6800_ca8k(void *dev_handle, S32 *freq_offset_KHz)
{
    *freq_offset_KHz = 0;

    return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_get_strength_tc6800_ca8k(void *dev_handle, U32 *p_gain, U32 *p_strength)
{
    MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;
    MT_FE_Tuner_Handle_TC6800 tuner_handle = (MT_FE_Tuner_Handle_TC6800)(handle->tuner_settings.tuner_handle);

    *p_gain = 0;

    //*p_strength = (U32)(mt_fe_tn_get_signal_strength_tc6800(tuner_handle) + 107.0);		// dBm + 107 ->dBuV
    *p_strength = (U32)(mt_fe_tn_get_signal_strength_tc6800(tuner_handle) + 107); // dBm + 107 ->dBuV

    return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_sleep_tc6800_ca8k(void *dev_handle)
{
    MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;
    MT_FE_Tuner_Handle_TC6800 tuner_handle = (MT_FE_Tuner_Handle_TC6800)(handle->tuner_settings.tuner_handle);

    mt_fe_tn_sleep_tc6800(tuner_handle);

    return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_wakeup_tc6800_ca8k(void *dev_handle)
{
    MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;
    MT_FE_Tuner_Handle_TC6800 tuner_handle = (MT_FE_Tuner_Handle_TC6800)(handle->tuner_settings.tuner_handle);

    mt_fe_tn_wakeup_tc6800(tuner_handle);

    return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_get_version_tc6800_ca8k(void *dev_handle, U32 *version_no, U32 *version_time)
{
    MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;
    MT_FE_Tuner_Handle_TC6800 tuner_handle = (MT_FE_Tuner_Handle_TC6800)(handle->tuner_settings.tuner_handle);

    *version_no = 0;
    *version_time = 0;
    if (!(handle->tuner_settings.tuner_init_OK)) {
	return MtFeErr_Uninit;
    }

    *version_no = tuner_handle->tuner_version;
    *version_time = tuner_handle->tuner_time;

    return MtFeErr_Ok;
}

#if 0
S32 mt_fe_ca8k_cab_tn_self_check_tc6800(void *dev_handle)
{
	S32 ret = 0;

	MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;
	MT_FE_Tuner_Handle_TC6800 tuner_handle = (MT_FE_Tuner_Handle_TC6800)(handle->tuner_settings.tuner_handle);

	if((tuner_handle == NULL) || (!tuner_handle->tuner_init_OK))
	{
		mt_fe_cs8k_cab_tn_init_tc6800(dev_handle);
		tuner_handle = (MT_FE_Tuner_Handle_TC6800)(handle->tuner_settings.tuner_handle);
	}

	ret = mt_fe_tn_self_check_tc6800(tuner_handle);

	return ret;
}
#endif

#if 0
/*  Rafael  R836    -       1.02.10 151019  15:00   YZ.Huang    Create       */
MT_FE_RET mt_fe_cs8k_cab_tn_init_R836(void *dev_handle)
{
	MT_FE_RET ret;
	MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;

	ret = mt_fe_tn_init_R836(handle);
	handle->tuner_settings.tuner_init_OK = (ret == MtFeErr_Ok) ? TRUE : FALSE;

	return ret;
}

MT_FE_RET mt_fe_cs8k_cab_tn_set_freq_R836(void *dev_handle, U32 freq_KHz, U16 symbol_rate_KSs)
{
	return mt_fe_tn_set_freq_R836(dev_handle, freq_KHz, symbol_rate_KSs);
}

MT_FE_RET mt_fe_cs8k_cab_tn_get_strength_R836(void *dev_handle, U32 *p_gain, U32 *p_strength)
{
	return mt_fe_tn_get_signal_strength_R836(dev_handle, p_gain, p_strength);
}

MT_FE_RET mt_fe_cs8k_cab_tn_get_version_R836(void *dev_handle, U32 *version_no, U32 *version_time)
{
	*version_no = 2095;
	*version_time = 150611;

	return MtFeErr_Ok;
}

S32 mt_fe_cs8k_cab_tn_self_check_R836(void *dev_handle)
{
	S32 ret = 0;

	MT_FE_CA8K_CAB_Device_Handle handle = (MT_FE_CA8K_CAB_Device_Handle)dev_handle;

	ret = mt_fe_tn_self_check_R836(handle);

	return ret;
}
#endif
