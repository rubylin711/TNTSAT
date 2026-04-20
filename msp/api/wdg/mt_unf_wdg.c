/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifdef CONFIG_MT_WDG_SUPPORT

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "drv_wdg_ioctl.h"
#include "mt_type.h"
#include "mt_error_mpi.h"
#include "mt_drv_struct.h"

static mt_s32 g_s32WDGDevFd = 0;

#define WATCHDOG_TIMEOUT_MAX 356000
#define WATCHDOG_TIMEOUT_MIN 1000

/*---- wdg ----*/

/*******************************************
Function:              mt_unf_wdg_Init
Description:   Init WDG devide
Data Accessed:  NA
Data Updated:   NA
Input:          NA
Output:         NA
Return:      ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_wdg_init(mt_void)
{
    mt_s32 s32DevFd = 0;

    if (g_s32WDGDevFd > 0)
    {
        return MT_SUCCESS;
    }

    s32DevFd = open("/dev/"UMAP_DEVNAME_WDG, O_RDWR | O_CLOEXEC, 0);

    if (s32DevFd < 0)
    {
        MT_ERR_WDG("open %s error\n", UMAP_DEVNAME_WDG);
        return MT_ERR_WDG_FAILED_INIT;
    }
    else
    {
        g_s32WDGDevFd = s32DevFd;
    }

    return MT_SUCCESS;
}

/*******************************************
Function:              mt_unf_wdg_deinit
Description:  Deinit WDG device
Data Accessed:  NA
Data Updated:   NA
Input:          NA
Output:         NA
Return:      ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_wdg_deinit(mt_void)
{
    mt_s32 Ret;

    if (g_s32WDGDevFd <= 0)
    {
        return MT_SUCCESS;
    }
    else
    {
		Ret = close(g_s32WDGDevFd);

	    if (MT_SUCCESS != Ret)
	    {
	        MT_FATAL_WDG("DeInit WDG err.\n");
	        return MT_ERR_WDG_FAILED_DEINIT;
	    }

        g_s32WDGDevFd = 0;
        return MT_SUCCESS;
    }
}

mt_s32 mt_unf_wdg_get_capability(mt_u32 *pu32WdgNum)
{
	if (MT_NULL != pu32WdgNum)
	{
		*pu32WdgNum = MT_WDG_NUM;
		return MT_SUCCESS;
	}

	return MT_FAILURE;
}

/*******************************************
Function:              mt_unf_wdg_enable
Description:  enable WDG device
Data Accessed:  NA
Data Updated:   NA
Input:          NA
Output:         NA
Return:      ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_wdg_enable(mt_u32 u32WdgNum)
{
    mt_s32 s32Result = 0;
	wdg_option_s stOption;

    if (g_s32WDGDevFd <= 0)
    {
        MT_ERR_WDG("file descriptor is illegal\n");
        return MT_ERR_WDG_NOT_INIT;
    }

	if (u32WdgNum >= MT_WDG_NUM)
    {
        MT_ERR_WDG("Input parameter(u32WdgNum) invalid: %d\n", u32WdgNum);
        return MT_ERR_WDG_INVALID_PARA;
    }

	stOption.s32Option = WDIOS_ENABLECARD;
	stOption.u32WdgIndex = u32WdgNum;

    s32Result = ioctl(g_s32WDGDevFd, WDIOC_SET_OPTIONS, &stOption);
    if (MT_SUCCESS != s32Result)
    {
        MT_ERR_WDG("wdg enable failed\n");
        return MT_ERR_WDG_FAILED_ENABLE;
    }
    else
    {
        return MT_SUCCESS;
    }
}


/*******************************************
Function:              mt_unf_wdg_enable_irq
Description:  enable WDG device irq
Data Accessed:  NA
Data Updated:   NA
Input:          NA
Output:         NA
Return:      ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_wdg_enable_irq(mt_u32 u32WdgNum)
{
    mt_s32 s32Result = 0;
	wdg_option_s stOption;

    if (g_s32WDGDevFd <= 0)
    {
        MT_ERR_WDG("file descriptor is illegal\n");
        return MT_ERR_WDG_NOT_INIT;
    }

	if (u32WdgNum >= MT_WDG_NUM)
    {
        MT_ERR_WDG("Input parameter(u32WdgNum) invalid: %d\n", u32WdgNum);
        return MT_ERR_WDG_INVALID_PARA;
    }

	stOption.s32Option = WDIOS_ENABLECARD_IRQ;
	stOption.u32WdgIndex = u32WdgNum;

    s32Result = ioctl(g_s32WDGDevFd, WDIOC_SET_OPTIONS, &stOption);
    if (MT_SUCCESS != s32Result)
    {
        MT_ERR_WDG("wdg enable failed\n");
        return MT_ERR_WDG_FAILED_ENABLE;
    }
    else
    {
        return MT_SUCCESS;
    }
}

/*******************************************
Function:              mt_unf_wdg_disable_irq
Description:  disable WDG device
Data Accessed:  NA
Data Updated:   NA
Input:          NA
Output:         NA
Return:      ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_wdg_disable_irq(mt_u32 u32WdgNum)
{
    mt_s32 s32Result = 0;
	wdg_option_s stOption;

    if (g_s32WDGDevFd <= 0)
    {
        MT_ERR_WDG("file descriptor is illegal\n");
        return MT_ERR_WDG_NOT_INIT;
    }

	if (u32WdgNum >= MT_WDG_NUM)
    {
        MT_ERR_WDG("Input parameter(u32WdgNum) invalid: %d\n", u32WdgNum);
        return MT_ERR_WDG_INVALID_PARA;
    }

	stOption.s32Option = WDIOS_DISABLECARD_IRQ;
	stOption.u32WdgIndex = u32WdgNum;

    s32Result = ioctl(g_s32WDGDevFd, WDIOC_SET_OPTIONS, &stOption);
    if (MT_SUCCESS != s32Result)
    {
        MT_ERR_WDG("wdg enable failed\n");
        return MT_ERR_WDG_FAILED_ENABLE;
    }
    else
    {
        return MT_SUCCESS;
    }
}


/*******************************************
Function:              mt_unf_wdg_disable
Description:  disable WDG device
Data Accessed:  NA
Data Updated:   NA
Input:          NA
Output:         NA
Return:      ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_wdg_disable(mt_u32 u32WdgNum)
{
    int s32Result = 0;
	wdg_option_s stOption;

    if (g_s32WDGDevFd <= 0)
    {
        MT_ERR_WDG("file descriptor is illegal\n");
        return MT_ERR_WDG_NOT_INIT;
    }

	if (u32WdgNum >= MT_WDG_NUM)
    {
        MT_ERR_WDG("Input parameter(u32WdgNum) invalid: %d\n", u32WdgNum);
        return MT_ERR_WDG_INVALID_PARA;
    }

	stOption.s32Option = WDIOS_DISABLECARD;
	stOption.u32WdgIndex = u32WdgNum;

    s32Result = ioctl(g_s32WDGDevFd, WDIOC_SET_OPTIONS, &stOption);

    if (MT_SUCCESS != s32Result)
    {
        MT_ERR_WDG("wdg disable failed\n");
        return MT_ERR_WDG_FAILED_DISABLE;
    }
    else
    {
        return MT_SUCCESS;
    }
}

/*******************************************
Function:              mt_unf_wdg_set_timeout
Description:  set the time interval of feeding the WDG
Calls:        MT_WDG_SetTimeout
Data Accessed:  NA
Data Updated:   NA
Input:          NA
Output:         NA
Return:      ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_wdg_set_timeout(mt_u32 u32WdgNum, mt_u32 u32Value)
{
    int s32Result = 0;
    //mt_u32 u32ValueInSec;
    wdg_timeout_s stTimeout;

    if ((u32WdgNum >= MT_WDG_NUM)
		|| (u32Value > WATCHDOG_TIMEOUT_MAX)
		|| (u32Value < WATCHDOG_TIMEOUT_MIN))
    {
        MT_ERR_WDG("Input parameter(u32Value) invalid: %d\n", u32Value);
        return MT_ERR_WDG_INVALID_PARA;
    }

    if (g_s32WDGDevFd <= 0)
    {
        MT_ERR_WDG("file descriptor is illegal\n");
        return MT_ERR_WDG_NOT_INIT;
    }

    /* convert ms to s */
    //u32ValueInSec = (u32Value + 999) / 1000;

	stTimeout.s32Timeout = (mt_s32)u32Value;
	stTimeout.u32WdgIndex = u32WdgNum;

    s32Result = ioctl(g_s32WDGDevFd, WDIOC_SET_TIMEOUT, &stTimeout);
    if (MT_SUCCESS != s32Result)
    {
        MT_ERR_WDG("wdg set timeout failed\n");
        return MT_ERR_WDG_FAILED_SETTIMEOUT;
    }
    else
    {
        return MT_SUCCESS;
    }
}

/*******************************************
Function:     mt_unf_wdg_get_timeout
Description:  get the time interval of feeding the WDG
Calls:        MT_WDG_GetTimeout
Data Accessed:  NA
Data Updated:   NA
Input:          NA
Output:         NA
Return:       ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_wdg_get_timeout(mt_u32 u32WdgNum, mt_u32 *pu32Value)
{
    mt_s32 s32Result = 0;
    wdg_timeout_s stTimeout;

    if (g_s32WDGDevFd <= 0)
    {
        MT_ERR_WDG("file descriptor is illegal\n");
        return MT_ERR_WDG_NOT_INIT;
    }

    if ((u32WdgNum >= MT_WDG_NUM) || (MT_NULL == pu32Value))
    {
        MT_ERR_WDG("para pu32Value is null.\n");
        return MT_ERR_WDG_INVALID_PARA;
    }

	stTimeout.u32WdgIndex = u32WdgNum;

    s32Result = ioctl(g_s32WDGDevFd, WDIOC_GET_TIMEOUT, &stTimeout);
    if (MT_SUCCESS != s32Result)
    {
        MT_ERR_WDG("wdg get timeout failed\n");
        return MT_ERR_WDG_FAILED_GETTIMEOUT;
    }
    else
    {
        /* convert s to ms */
        *pu32Value = (mt_u32)(stTimeout.s32Timeout * 1000);
        return MT_SUCCESS;
    }
}

/*******************************************
Function:              mt_unf_wdg_clear
Description:  clear the WDG
Calls:        MT_WDG_ClearWatchDog
Data Accessed:  NA
Data Updated:   NA
Input:          NA
Output:         NA
Return:      ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_wdg_clear(mt_u32 u32WdgNum)
{
    mt_s32 s32Result = 0;

    if (g_s32WDGDevFd <= 0)
    {
        MT_ERR_WDG("file descriptor is illegal\n");
        return MT_ERR_WDG_NOT_INIT;
    }

	if (u32WdgNum >= MT_WDG_NUM)
    {
        MT_ERR_WDG("Input parameter(u32WdgNum) invalid: %d\n", u32WdgNum);
        return MT_ERR_WDG_INVALID_PARA;
    }

    s32Result = ioctl(g_s32WDGDevFd, WDIOC_KEEP_ALIVE, &u32WdgNum);

    if (MT_SUCCESS != s32Result)
    {
        MT_ERR_WDG("clear wdg failed\n");
        return MT_ERR_WDG_FAILED_CLEARWDG;
    }
    else
    {
        return MT_SUCCESS;
    }
}

/*******************************************
Function:              mt_unf_wdg_reset
Description:  reset WDG
Calls:        MT_WDG_Reset
Data Accessed:  NA
Data Updated:   NA
Input:          NA
Output:         NA
Return:      ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_wdg_reset(mt_u32 u32WdgNum)
{
    mt_s32 s32Result = 0;
    wdg_option_s stOption;

    if (g_s32WDGDevFd <= 0)
    {
        MT_ERR_WDG("file descriptor is illegal\n");
        return MT_ERR_WDG_NOT_INIT;
    }

	if (u32WdgNum >= MT_WDG_NUM)
    {
        MT_ERR_WDG("Input parameter(u32WdgNum) invalid: %d\n", u32WdgNum);
        return MT_ERR_WDG_INVALID_PARA;
    }

	stOption.s32Option = WDIOS_RESET_BOARD;
	stOption.u32WdgIndex = u32WdgNum;

    s32Result = ioctl(g_s32WDGDevFd, WDIOC_SET_OPTIONS, &stOption);

    if (MT_SUCCESS != s32Result)
    {
        MT_ERR_WDG("reset faile\n");
        return MT_ERR_WDG_FAILED_RESET;
    }
    else
    {
        return MT_SUCCESS;
    }
}



#else

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/watchdog.h>

#include <fcntl.h>
#include <unistd.h>

#include "drv_wdg_ioctl.h"
#include "mt_type.h"
#include "mt_error_mpi.h"
#include "mt_drv_struct.h"


#define WDG_MAX_NUM				(2)
#define WATCHDOG_TIMEOUT_MAX	356000
#define WATCHDOG_TIMEOUT_MIN	1000

static mt_s32 g_s32WDGDevFd[WDG_MAX_NUM];


/*******************************************
Function:			mt_unf_wdg_Init
Description:		Init WDG devide
Return:				ErrorCode(reference to document)
*******************************************/
mt_s32 mt_unf_wdg_init(mt_void)
{
    mt_s32 s32DevFd = 0;

    if (g_s32WDGDevFd[0] > 0)
    {
        return MT_SUCCESS;
    }
#if 0
	switch (0)
	{
		case 0:
			//s32DevFd = open("/dev/watchdog0", O_RDWR, 0);//O_RDWR//O_WRONLY
			s32DevFd = open("/dev/watchdog0", O_WRONLY);//O_RDWR//O_WRONLY
			break;
		case 1:
			s32DevFd = open("/dev/watchdog1", O_WRONLY);
			break;
		default:
			s32DevFd = open("/dev/watchdog", O_WRONLY);
			break;
			//return MT_ERR_WDG_INVALID_PARA;
	}
#endif

	s32DevFd = open("/dev/watchdog0", O_WRONLY);//O_RDWR//O_WRONLY
    if (s32DevFd < 0)
    {
        MT_ERR_WDG("\n###[watchdog] watchdog open error\n");
        return MT_ERR_WDG_FAILED_INIT;
    }
    else
    {
        g_s32WDGDevFd[0] = s32DevFd;
    }

    return MT_SUCCESS;
}

/*******************************************
Function:				mt_unf_wdg_deinit
Description:			Deinit WDG device
Return:					ErrorCode(reference to document)
*******************************************/
mt_s32 mt_unf_wdg_deinit(mt_void)
{
    mt_s32 Ret;

    if (g_s32WDGDevFd[0] <= 0)
    {
        return MT_SUCCESS;
    }
    else
    {
		Ret = close(g_s32WDGDevFd[0]);

	    if (MT_SUCCESS != Ret)
	    {
	        MT_FATAL_WDG("DeInit WDG err.\n");
	        return MT_ERR_WDG_FAILED_DEINIT;
	    }

        g_s32WDGDevFd[0] = 0;
        return MT_SUCCESS;
    }
}

mt_s32 mt_unf_wdg_get_capability(mt_u32 *pu32WdgNum)
{

	if (MT_NULL != pu32WdgNum)
	{
		*pu32WdgNum = MT_WDG_NUM;
		return MT_SUCCESS;
	}

	return MT_FAILURE;
}

/*******************************************
Function:				mt_unf_wdg_enable
Description:			enable WDG device
Return:					ErrorCode(reference to document)
*******************************************/
mt_s32 mt_unf_wdg_enable(mt_u32 u32WdgNum)
{
    mt_s32 s32Result = 0;
	mt_u32 wdt_option = WDIOS_ENABLECARD;

	if (WDG_MAX_NUM <= u32WdgNum)
	{
		MT_ERR_WDG("Input parameter(u32WdgNum) invalid: %d\n", u32WdgNum);
		return MT_ERR_WDG_INVALID_PARA;
	}

    if (g_s32WDGDevFd[u32WdgNum] <= 0)
    {
        MT_ERR_WDG("file descriptor is illegal\n");
        return MT_ERR_WDG_NOT_INIT;
    }

    s32Result = ioctl(g_s32WDGDevFd[u32WdgNum], WDIOC_SETOPTIONS, &wdt_option);//WDIOC_SETOPTIONS
    if (MT_SUCCESS != s32Result)
    {
        MT_ERR_WDG("wdg enable failed\n");
        return MT_ERR_WDG_FAILED_ENABLE;
    }
    else
    {
        return MT_SUCCESS;
    }
}


/*******************************************
Function:				mt_unf_wdg_disable
Description:			disable WDG device
Return:					ErrorCode(reference to document)
*******************************************/
mt_s32 mt_unf_wdg_disable(mt_u32 u32WdgNum)
{
    int s32Result = 0;
	mt_u32 wdt_option = WDIOS_DISABLECARD;

	if (WDG_MAX_NUM <= u32WdgNum)
	{
		MT_ERR_WDG("Input parameter(u32WdgNum) invalid: %d\n", u32WdgNum);
		return MT_ERR_WDG_INVALID_PARA;
	}

    if (g_s32WDGDevFd[u32WdgNum] <= 0)
    {
        MT_ERR_WDG("file descriptor is illegal\n");
        return MT_ERR_WDG_NOT_INIT;
    }

	if (u32WdgNum >= MT_WDG_NUM)
    {
        MT_ERR_WDG("Input parameter(u32WdgNum) invalid: %d\n", u32WdgNum);
        return MT_ERR_WDG_INVALID_PARA;
    }

    s32Result = ioctl(g_s32WDGDevFd[u32WdgNum], WDIOC_SETOPTIONS, &wdt_option);

    if (MT_SUCCESS != s32Result)
    {
        MT_ERR_WDG("wdg disable failed\n");
        return MT_ERR_WDG_FAILED_DISABLE;
    }
    else
    {
        return MT_SUCCESS;
    }
}

/*******************************************
Function:				mt_unf_wdg_set_timeout
Description:			set the time interval of feeding the WDG
Calls:					MT_WDG_SetTimeout
Return:					ErrorCode(reference to document)
*******************************************/
mt_s32 mt_unf_wdg_set_timeout(mt_u32 u32WdgNum, mt_u32 u32Value)
{
    int s32Result = 0;
    //mt_u32 u32ValueInSec;
    mt_u32 wdt_timeout = 0;

    if ((u32WdgNum >= MT_WDG_NUM)
		|| (u32Value > WATCHDOG_TIMEOUT_MAX)
		|| (u32Value < WATCHDOG_TIMEOUT_MIN))
    {
        MT_ERR_WDG("Input parameter(u32Value) invalid: %d\n", u32Value);
        return MT_ERR_WDG_INVALID_PARA;
    }

    if (g_s32WDGDevFd[u32WdgNum] <= 0)
    {
        MT_ERR_WDG("file descriptor is illegal\n");
        return MT_ERR_WDG_NOT_INIT;
    }

    /* convert ms to s */
    //u32ValueInSec = (u32Value + 999) / 1000;

	wdt_timeout = u32Value;

    s32Result = ioctl(g_s32WDGDevFd[u32WdgNum], WDIOC_SETTIMEOUT, &wdt_timeout);
    if (MT_SUCCESS != s32Result)
    {
        MT_ERR_WDG("wdg set timeout failed\n");
        return MT_ERR_WDG_FAILED_SETTIMEOUT;
    }
    else
    {
        return MT_SUCCESS;
    }
}

/*******************************************
Function:				mt_unf_wdg_get_timeout
Description:			get the time interval of feeding the WDG
Calls:					MT_WDG_GetTimeout
Return:					ErrorCode(reference to document)
*******************************************/
mt_s32 mt_unf_wdg_get_timeout(mt_u32 u32WdgNum, mt_u32 *pu32Value)
{
    mt_s32 s32Result = 0;
    mt_u32 wdt_timeout = 0;

    if ((u32WdgNum >= MT_WDG_NUM) || (MT_NULL == pu32Value))
    {
        MT_ERR_WDG("para pu32Value is null.\n");
        return MT_ERR_WDG_INVALID_PARA;
    }

    if (g_s32WDGDevFd[u32WdgNum] <= 0)
    {
        MT_ERR_WDG("file descriptor is illegal\n");
        return MT_ERR_WDG_NOT_INIT;
    }

    s32Result = ioctl(g_s32WDGDevFd[u32WdgNum], WDIOC_GETTIMEOUT, &wdt_timeout);
    if (MT_SUCCESS != s32Result)
    {
        MT_ERR_WDG("wdg get timeout failed\n");
        return MT_ERR_WDG_FAILED_GETTIMEOUT;
    }
    else
    {
        /* convert s to ms */
        *pu32Value = (mt_u32)(wdt_timeout * 1000);
        return MT_SUCCESS;
    }
}

/*******************************************
Function:				mt_unf_wdg_clear
Description:			clear the WDG
Calls:					MT_WDG_ClearWatchDog
Return:					ErrorCode(reference to document)
*******************************************/
mt_s32 mt_unf_wdg_clear(mt_u32 u32WdgNum)
{
    mt_s32 s32Result = 0;

	if (WDG_MAX_NUM <= u32WdgNum)
	{
		MT_ERR_WDG("Input parameter(u32WdgNum) invalid: %d\n", u32WdgNum);
		return MT_ERR_WDG_INVALID_PARA;
	}

    if (g_s32WDGDevFd[u32WdgNum] <= 0)
    {
        MT_ERR_WDG("file descriptor is illegal\n");
        return MT_ERR_WDG_NOT_INIT;
    }

    s32Result = ioctl(g_s32WDGDevFd[u32WdgNum], WDIOC_KEEPALIVE, 0);

    if (MT_SUCCESS != s32Result)
    {
        MT_ERR_WDG("clear wdg failed\n");
        return MT_ERR_WDG_FAILED_CLEARWDG;
    }
    else
    {
        return MT_SUCCESS;
    }
}

/*******************************************
Function:				mt_unf_wdg_reset
Description:			reset WDG
Calls:					MT_WDG_Reset
Return:					ErrorCode(reference to document)
*******************************************/
mt_s32 mt_unf_wdg_reset(mt_u32 u32WdgNum)
{
    mt_s32 s32Result = 0;
    mt_u32 wdt_option = 0x0008;//WDIOS_DISABLECARD;WDIOS_RESET_BOARD

    if (g_s32WDGDevFd[u32WdgNum] <= 0)
    {
        MT_ERR_WDG("file descriptor is illegal\n");
        return MT_ERR_WDG_NOT_INIT;
    }

	if (u32WdgNum >= MT_WDG_NUM)
    {
        MT_ERR_WDG("Input parameter(u32WdgNum) invalid: %d\n", u32WdgNum);
        return MT_ERR_WDG_INVALID_PARA;
    }

    s32Result = ioctl(g_s32WDGDevFd[u32WdgNum], WDIOC_SETOPTIONS, &wdt_option);
    if (MT_SUCCESS != s32Result)
    {
        MT_ERR_WDG("reset faile\n");
        return MT_ERR_WDG_FAILED_RESET;
    }
    else
    {
        return MT_SUCCESS;
    }
}


/*******************************************
Function:				mt_unf_wdg_enable_irq
Description:			enable WDG device irq
Return:					ErrorCode(reference to document)
*******************************************/
mt_s32 mt_unf_wdg_enable_irq(mt_u32 u32WdgNum)
{
	MT_ERR_WDG("\n TODO.\n");
	return MT_FAILURE;
}

/*******************************************
Function:				mt_unf_wdg_disable_irq
Description:			disable WDG device irq
Return:					ErrorCode(reference to document)
*******************************************/
mt_s32 mt_unf_wdg_disable_irq(mt_u32 u32WdgNum)
{
	MT_ERR_WDG("\n TODO.\n");
	return MT_FAILURE;
}

#endif
