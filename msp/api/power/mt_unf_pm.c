/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#include <pthread.h>
#include "signal.h"
#include "semaphore.h"
#include "drv_pm_ioctl.h"
#include "mt_unf_pm.h"

static int PmHandle = -1;

MT_S32 MT_UNF_PMOC_SwitchSystemMode(void)
{
	int ret = 0;

	ret = ioctl(PmHandle, CMD_PM_IN_LOW_POWER, 0);
	if(ret < 0)
	{
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}


MT_S32 MT_UNF_PMOC_SetStandbyDispMode(sty_disp_conf_t dconf)
{
	int ret = 0;
	ret = ioctl(PmHandle, CMD_PM_SET_DISP_MODE, &dconf);
	if(ret < 0)
	{
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}


MT_S32 MT_UNF_PMOC_SetWakeUpAttr(sty_wakeup_conf_t  wconf)
{
	int ret = 0;
	ret = ioctl(PmHandle, CMD_PM_SET_WAKEUP_MODE, &wconf);
	if(ret < 0)
	{
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}


MT_S32 MT_UNF_PMOC_SetDevType( sty_fp_type_t dev_type)
{
	int ret = 0;
	ret = ioctl(PmHandle, CMD_PM_SET_FPDEV_TYPE, dev_type);
	if(ret < 0)
	{
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}


MT_S32 MT_UNF_PMOC_ConfigParams(sty_param_conf_t config)
{
	int ret = 0;
	ret = ioctl(PmHandle, CMD_PM_SET_PARAM, &config);
	if(ret < 0)
	{
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}


MT_S32 MT_UNF_PMOC_GetStandbyTime(standby_time_t *ptime)
{
	int ret = 0;
	standby_time_t time = {0};

	ret = ioctl(PmHandle, CMD_PM_GET_STANDBY_TIME, &time);
	if((ret < 0) || (NULL == ptime))
	{
		return MT_FAILURE;
	}

	ptime->pass_day = time.pass_day;
	ptime->cur_hour = time.cur_hour;
	ptime->cur_min = time.cur_min;
	ptime->cur_sec = time.cur_sec;

	return MT_SUCCESS;
}


MT_S32 MT_UNF_PMOC_GetStandbyInfo(MT_U32 *pinfo)
{
	int ret = 0;
	mt_u32 finfo = 0;

	ret = ioctl(PmHandle, CMD_PM_GET_STANDBY_INFO, &finfo);
	if((ret < 0) || (NULL == pinfo))
	{
		return MT_FAILURE;
	}

	*pinfo = finfo;

	return MT_SUCCESS;
}


MT_S32 MT_UNF_PMOC_GetDispMode(sty_disp_conf_t *pdconf)
{
	int ret = 0;
	sty_disp_conf_t finfo = {0};

	ret = ioctl(PmHandle, CMD_PM_GET_DISP_MODE, &finfo);
	if((ret < 0) || (NULL == pdconf))
	{
		return MT_FAILURE;
	}

	memcpy(pdconf, &finfo, sizeof(sty_disp_conf_t));

	return MT_SUCCESS;
}

MT_S32 MT_UNF_PMOC_Init(void)
{
	if(PmHandle != -1)
		return MT_SUCCESS;

	PmHandle = open ("/dev/mt_pm",  O_RDWR | O_CLOEXEC);
	if(PmHandle < 0)
	{
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}


MT_S32 MT_UNF_PMOC_DeInit(void)
{
	if(PmHandle != -1)
	{
		close(PmHandle);
		PmHandle = -1;
	}

	return MT_SUCCESS;
}


MT_S32 MT_UNF_PMOC_SetGpenPin( sty_gpen_pin_t  val)
{
	int ret = 0;
	ret = ioctl(PmHandle, CMD_PM_SET_GPEN, val);
	if(ret < 0)
	{
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

MT_S32 MT_UNF_PMOC_SwitchOSCClock(void)
{
	int ret = 0;
	ret = ioctl(PmHandle, CMD_PM_SWITCH_OSC_CLOCK, 0);
	if(ret < 0)
	{
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

MT_S32 MT_UNF_PMOC_SetCecConfig(cec_config_t  cec_cfg)
{
	int ret = 0;
	ret = ioctl(PmHandle, CMD_PM_CEC_CONFIG, &cec_cfg);
	if(ret < 0)
	{
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

MT_S32 MT_UNF_PMOC_SetUnixTime(MT_U32 time)
{
	int ret = 0;
	MT_U32 set_time = time;

	ret = ioctl(PmHandle, CMD_PM_SET_UNIX_TIME, &set_time);
	if(ret < 0)
	{
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

MT_S32 MT_UNF_PMOC_GetUnixTime(MT_U32  *time)
{
	int ret = 0;
	MT_U32 get_time = 0;
	ret = ioctl(PmHandle, CMD_PM_GET_UNIX_TIME, &get_time);
	if((ret < 0) || (NULL == time))
	{
		return MT_FAILURE;
	}

    *time = get_time;
	return MT_SUCCESS;
}

MT_S32 MT_UNF_PMOC_ClearStandbyInfo(void)
{
	int ret = 0;
	//mt_u32 finfo = 0;

	ret = ioctl(PmHandle, CMD_PM_CLEAR_STANDBY_INFO, NULL);
	if(ret < 0)
	{
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

MT_S32 MT_UNF_PMOC_SetWakeUpConfig(sty_wakeup_display_t  wconf)
{
	int ret = 0;
	ret = ioctl(PmHandle, CMD_PM_SET_WAKEUP_CONFIG, &wconf);
	if(ret < 0)
	{
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

MT_S32 MT_UNF_PMOC_SetBleWakeup(sty_wakeup_ble_conf_t ble_cfg)
{
	int ret = 0;
	ret = ioctl(PmHandle, CMD_PM_SET_BLE_WAKEUP, &ble_cfg);
	if(ret < 0)
	{
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

MT_S32 MT_UNF_PMOC_KadcConfig(standby_kadc_config_t kadc_cfg)
{
	int ret = 0;

	if(PmHandle <= 0)
	{
		return MT_FAILURE;
	}

	ret = ioctl(PmHandle, CMD_PM_KADC_CONFIG, &kadc_cfg);
	if(ret < 0)
	{
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

MT_S32 MT_UNF_PMOC_ExternAomcuConfig(sty_aomcu_fw_conf_t aomcu_info)
{
	int ret = 0;

	if(PmHandle <= 0)
	{
		return MT_FAILURE;
	}

	ret = ioctl(PmHandle, CMD_PM_AO_FW_CONFIG, &aomcu_info);
	if(ret < 0)
	{
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

