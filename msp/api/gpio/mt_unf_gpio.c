/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <pthread.h>


#include "mt_type.h"
#include "drv_gpio_ioctl.h"
#include "mt_unf_gpio.h"

static int GpioHandle = -1;

static MT_S32 MT_MPI_GPIO_Io_Enable(MT_UNF_GPIO_LIST_E u32GpioNo, MT_BOOL bEnable)
{
	int i32RetVal = 0;
	struct gpio_mask_param stGpioEnable;

	if (bEnable)
	{
		stGpioEnable.val = GPIO_MASK_ENABLE;
	}
	else
	{
		stGpioEnable.val = GPIO_MASK_DISABLE;
	}

	stGpioEnable.gpio = u32GpioNo;

	i32RetVal = ioctl(GpioHandle, GPIO_DRV_IOC_IO_ENABLE, &stGpioEnable);
	if(i32RetVal < 0)
		return MT_FAILURE;

	return MT_SUCCESS;
}

MT_S32 MT_UNF_GPIO_Init(MT_VOID)
{
	if(GpioHandle != -1)
		return MT_SUCCESS;

	GpioHandle = open ("/dev/mt_gpio",  O_RDWR | O_CLOEXEC);
	if(GpioHandle < 0)
	{
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

MT_S32 MT_UNF_GPIO_Deinit(MT_VOID)
{
	if(GpioHandle != -1)
	{
		close(GpioHandle);
		GpioHandle = -1;
	}

	return MT_SUCCESS;
}

MT_S32 MT_UNF_GPIO_SetDirBit(MT_UNF_GPIO_LIST_E u32GpioNo, MT_BOOL bInput)
{
	struct gpio_dir_param stGpioDir;
	int i32RetVal = 0;

	if (GpioHandle == -1)
		return MT_FAILURE;

	//Enable first!
	i32RetVal = MT_MPI_GPIO_Io_Enable(u32GpioNo, MT_TRUE);
	if(i32RetVal < 0)
		return MT_FAILURE;

	if(bInput == MT_TRUE)
	{
		stGpioDir.dir = GPIO_DIR_INPUT;
	}
	else
	{
		stGpioDir.dir = GPIO_DIR_OUTPUT;
	}

	stGpioDir.gpio = u32GpioNo;

	i32RetVal = ioctl(GpioHandle, GPIO_DRV_IOC_SET_DIR, &stGpioDir);
	if(i32RetVal < 0)
		return MT_FAILURE;

	return MT_SUCCESS;
}

MT_S32 MT_UNF_GPIO_GetDirBit(MT_UNF_GPIO_LIST_E u32GpioNo, MT_BOOL *pbInput)
{
	struct gpio_dir_param stGpioDir;
	int i32RetVal = 0;

	if (GpioHandle == -1)
		return MT_FAILURE;

	stGpioDir.gpio = u32GpioNo;
	i32RetVal = ioctl(GpioHandle, GPIO_DRV_IOC_GET_DIR, &stGpioDir);
	if(i32RetVal < 0)
		return MT_FAILURE;

	if(stGpioDir.dir == GPIO_DIR_INPUT)
	{
		*pbInput = MT_TRUE;
	}
	else
	{
		*pbInput = MT_FALSE;
	}

	return MT_SUCCESS;
}

MT_S32 MT_UNF_GPIO_WriteBit(MT_UNF_GPIO_LIST_E u32GpioNo, MT_BOOL bHighVolt)
{
	struct gpio_value_param stGpioValue;
	int i32RetVal = 0;

	if (GpioHandle == -1)
		return MT_FAILURE;

	if(bHighVolt == MT_TRUE)
	{
		stGpioValue.val = GPIO_VALUE_HIGH_LEVEL;
	}
	else
	{
		stGpioValue.val = GPIO_VALUE_LOW_LEVEL;
	}
	stGpioValue.gpio = u32GpioNo;

	i32RetVal = ioctl(GpioHandle, GPIO_DRV_IOC_SET_VALUE, &stGpioValue);
	if(i32RetVal < 0)
		return MT_FAILURE;

	return MT_SUCCESS;
}

MT_S32 MT_UNF_GPIO_ReadBit(MT_UNF_GPIO_LIST_E u32GpioNo, MT_BOOL *pbHighVolt)
{
	struct gpio_value_param stGpioValue;
	int i32RetVal = 0;

	if (GpioHandle == -1)
		return MT_FAILURE;

	stGpioValue.gpio = u32GpioNo;
	i32RetVal = ioctl(GpioHandle, GPIO_DRV_IOC_GET_VALUE, &stGpioValue);
	if(i32RetVal < 0)
		return MT_FAILURE;

	if(stGpioValue.val == GPIO_VALUE_HIGH_LEVEL)
	{
		*pbHighVolt = MT_TRUE;
	}
	else
	{
		*pbHighVolt = MT_FALSE;
	}

	return MT_SUCCESS;
}

