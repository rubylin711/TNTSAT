/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include "mt_fe_def_cs8000_cab.h"
#include "mt_fe_i2c.h"
#include "./MXL608/MxL608_TunerApi.h"
#include "./MXL608/MxL608_OEM_Drv.h"


MT_FE_RET mt_fe_tn_init_MxL608(void *dev_handle);
MT_FE_RET mt_fe_tn_set_freq_MxL608(void *dev_handle, U32 freq_KHz, U16 symbol_rate_KSs);
S32 mt_fe_tn_self_check_MxL608(MT_FE_CS8000_CAB_Device_Handle dev_handle);
MT_FE_RET mt_fe_tn_get_signal_strength_MxL608(void *dev_handle, U32 *p_gain, U32 *p_strength);


