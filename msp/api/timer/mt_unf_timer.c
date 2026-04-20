/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#include "mt_type.h"
#include "mt_error_mpi.h"
#include "mt_drv_struct.h"
#include "drv_timer.h"
#include "mt_unf_timer.h"

static mt_s32 g_s32TimerDevFd = 0;

#ifdef CONFIG_MT_CHIP_SYMPHONY4
#define M_SYS_TIMER_MAX_NUM    (8)
#elif CONFIG_MT_CHIP_SYMPHONY6
#define M_SYS_TIMER_MAX_NUM    (8)
#else
#define M_SYS_TIMER_MAX_NUM    (4)
#endif


/*******************************************
Function:              mt_unf_timer_Init
Description:   Init Timer devide
Data Accessed:  NA
Data Updated:   NA
Input:          NA
Output:         NA
Return:      ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_timer_init(mt_s32 *ps32TimerDevFd)
{
    mt_s32 s32DevFd = 0;

    if (g_s32TimerDevFd > 0)
    {
        return MT_SUCCESS;
    }

    if (MT_NULL == ps32TimerDevFd)
    {
        MT_ERR_TIMER("para pu32Value is null.\n");
        return MT_ERR_TIMER_INVALID_POINT;
    }

    s32DevFd = open("/dev/"UMAP_DEVNAME_TIMER, O_RDWR, 0);

    if (s32DevFd < 0)
    {
        MT_ERR_TIMER("open %s error\n", UMAP_DEVNAME_TIMER);
        return MT_ERR_TIMER_FAILED_INIT;
    }
    else
    {
        g_s32TimerDevFd = s32DevFd;
        *ps32TimerDevFd = s32DevFd;
    }

    return MT_SUCCESS;
}

/*******************************************
Function:              mt_unf_timer_deinit
Description:  Deinit Timer device
Data Accessed:  NA
Data Updated:   NA
Input:          NA
Output:         NA
Return:      ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_timer_deinit(mt_void)
{
    mt_s32 Ret;

    if (g_s32TimerDevFd <= 0)
    {
        return MT_SUCCESS;
    }
    else
    {
		Ret = close(g_s32TimerDevFd);

	    if (MT_SUCCESS != Ret)
	    {
	        MT_ERR_TIMER("DeInit WDG err.\n");
	        return MT_ERR_TIMER_FAILED_DEINIT;
	    }

        g_s32TimerDevFd = 0;
        return MT_SUCCESS;
    }
}


/*******************************************
Function:              mt_unf_timer_RELEASE
Description:  release timer
Data Accessed:  NA
Data Updated:   NA
Input:          Timer id
Output:         NA
Return:      ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_timer_release(mt_u32 u32TimerID)
{
    mt_s32 s32Result = 0;

    if (g_s32TimerDevFd <= 0)
    {
        MT_ERR_TIMER("file descriptor is illegal\n");
        return MT_ERR_TIMER_NOT_INIT;
    }

    if (u32TimerID >= M_SYS_TIMER_MAX_NUM)
    {
        MT_ERR_TIMER("Input parameter(u32WdgNum) invalid: %d\n", u32TimerID);
        return MT_ERR_TIMER_INVALID_ID;
    }

    s32Result = ioctl(g_s32TimerDevFd, TIMERIOC_SET_RELEASE, &u32TimerID);
    if (MT_SUCCESS != s32Result)
    {
        MT_ERR_TIMER("timer release failed\n");
        return MT_ERR_TIMER_FAILED_RELEASEL;
    }
    else
    {
        return MT_SUCCESS;
    }
}


/*******************************************
Function:              mt_unf_timer_RELEASE
Description:  release timer
Data Accessed:  NA
Data Updated:   NA
Input:          Timer id
Output:         NA
Return:      ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_timer_read_cnt(mt_u32 u32TimerID, mt_u32 *pu32Value)
{
    mt_s32 s32Result = 0;
    timer_cnt_s gtOption = {0};

    if (g_s32TimerDevFd <= 0)
    {
        MT_ERR_TIMER("file descriptor is illegal\n");
        return MT_ERR_TIMER_NOT_INIT;
    }

    if (u32TimerID >= M_SYS_TIMER_MAX_NUM)
    {
        MT_ERR_TIMER("Input parameter(u32TimerID) invalid: %d\n", u32TimerID);
        return MT_ERR_TIMER_INVALID_ID;
    }

     if (MT_NULL == pu32Value)
    {
        MT_ERR_TIMER("para pu32Value is null.\n");
        return MT_ERR_TIMER_INVALID_POINT;
    }

    gtOption.u32cnt = 0;
    gtOption.u32TimerIndex = u32TimerID;

    s32Result = ioctl(g_s32TimerDevFd, TIMERIOC_GET_CNT, &gtOption);
    if (MT_SUCCESS != s32Result)
    {
        MT_ERR_TIMER("timer read cnt failed\n");
        return MT_ERR_TIMER_GETCNT_FAILED;
    }
    else
    {
        *pu32Value = (mt_u32)gtOption.u32cnt;
        return MT_SUCCESS;
    }
}


/*******************************************
Function:              mt_unf_timer_request
Description:  request a timer
Data Accessed:  NA
Data Updated:   NA
Input:
Output:      timer id
Return:      ErrorCode(reference to document)
Others:         NA
*******************************************/
mt_s32 mt_unf_timer_request(mt_u32 u32Timerms, mt_u32 cycled, mt_u32 *pu32TimerID)
{
    mt_s32 s32Result = 0;
    mt_symp_timer_info stOption;

    if (g_s32TimerDevFd <= 0)
    {
        MT_ERR_TIMER("file descriptor is illegal\n");
        return MT_ERR_TIMER_NOT_INIT;
    }

    if (u32Timerms == 0)
    {
        MT_ERR_TIMER("Input parameter(u32Timerms) invalid: %d\n", u32Timerms);
        return MT_ERR_TIMER_INVALID_PARA;
    }

    stOption.timeout = (int)u32Timerms;
    stOption.cycled = (int)cycled;

    s32Result = ioctl(g_s32TimerDevFd, TIMERIOC_REQUEST, &stOption);
    if (MT_SUCCESS != s32Result)
    {
        MT_ERR_TIMER("timer request failed\n");
        return MT_ERR_TIMER_REQUEST_FAILED;
    }
    else
    {
        *pu32TimerID = (mt_u32)stOption.timer_id;
        return MT_SUCCESS;
    }
}


