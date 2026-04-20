/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include "mt_adp_hdmi.h"
#include "mt_adp_mpi.h"
#include "mt_unf_hdmi.h"
#include "mt_unf_disp.h"
#include "mt_unf_timer.h"
/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_STANDBY_TIMER_DEBUG
#define MT_STANDBY_TIMER_PRINT   printf
#else
#define MT_STANDBY_TIMER_PRINT
#endif

#define SAMPLE_STANDBY_TIMER_FUNCTION_ENTER()       MT_STANDBY_TIMER_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_STANDBY_TIMER_FUNCTION_EXIT()        MT_STANDBY_TIMER_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_STANDBY_TIMER_FATAL_PRINT(fmt...)    MT_STANDBY_TIMER_PRINT(" [FATAL] " fmt)
#define SAMPLE_STANDBY_TIMER_ERR_PRINT(fmt...)      MT_STANDBY_TIMER_PRINT(" [ERROR] " fmt)
#define SAMPLE_STANDBY_TIMER_WARN_PRINT(fmt...)     MT_STANDBY_TIMER_PRINT(" [WARN] "  fmt)
#define SAMPLE_STANDBY_TIMER_INFO_PRINT(fmt...)     MT_STANDBY_TIMER_PRINT(" [INFO] "  fmt)
#define SAMPLE_STANDBY_TIMER_DBG_PRINT(fmt...)      MT_STANDBY_TIMER_PRINT(" [DEBUG] " fmt)
/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/
static MT_U32 g_timer_id = -1;
static MT_S32 g_s32TimerDevFd;
static MT_S32 g_standby;
/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_StandbyTimerMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

/*!
@brief The timer interrupts the service function, and when the time is up, call this function to wake up again
@param[in]  hAvplay           Signal type
@return::MT_VOID
@*/
static MT_VOID MT_StandbyTimerModeTimerDefaultCallback(MT_S32 sig)
{
    MT_S32                   Ret = MT_FAILURE;
    MT_UNF_HDMI_OPEN_PARA_S  DefaultMode = { 0 };

    if(sig == SIGIO)
    {
        Ret = MT_UNF_DISP_SetHdVideoEnable(MT_TRUE);
        Ret |= MT_UNF_DISP_SetSdVideoEnable(MT_TRUE);
        Ret |= MT_UNF_HDMI_Open(MT_UNF_HDMI_ID_0, &DefaultMode);

        Ret |= MT_UNF_HDMI_Output_Set(MT_UNF_HDMI_ID_0, MT_TRUE);
        Ret |= MT_UNF_HDMI_Start(MT_UNF_HDMI_ID_0);
        Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_0, MT_TRUE);
        Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_1, MT_TRUE);
        Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_2, MT_TRUE);
        Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_3, MT_TRUE);
        if(Ret != MT_SUCCESS)
        {
            SAMPLE_STANDBY_TIMER_ERR_PRINT("Wake failed");
            Ret = mt_unf_timer_release(g_timer_id);
            g_timer_id = (MT_U32)-1;
            if (MT_SUCCESS != Ret)
            {
                SAMPLE_STANDBY_TIMER_ERR_PRINT("mt_unf_timer_release failed, ret = %d\n", Ret);
            }
        }
        Ret = mt_unf_timer_release(g_timer_id);
        g_timer_id = (MT_U32)-1;
        if (MT_SUCCESS != Ret)
        {
            SAMPLE_STANDBY_TIMER_ERR_PRINT("mt_unf_timer_release failed, ret = %d\n", Ret);
        }
        MT_STANDBY_TIMER_PRINT("\n");
        SAMPLE_STANDBY_TIMER_INFO_PRINT("From standby to wake-up\n");
    }
    else
    {
        SAMPLE_STANDBY_TIMER_INFO_PRINT("Did not receive SIGIO\n");
    }

    g_standby = MT_FALSE;
}


/*!
@brief Apply and set the timer and then enter standby
@param[in]  interval            Standby time
@return::MT_SUCCESS             Success.
@return::Ret                    Fail.
@*/
static MT_S32 MT_StandbyTimerModeSetParam(MT_U32 interval)
{
    MT_S32 Ret = MT_FAILURE;

    interval = interval * 1000;
    g_standby = MT_TRUE;
    SAMPLE_STANDBY_TIMER_FUNCTION_ENTER();

    /*
    Request a timer
    param[0] Timed time(in ms)
    param[1] Whether the timer needs to work in a loop, 0: Work once, 1: Cyclic work
    param[2] The timer ID got from request
    */
    Ret = mt_unf_timer_request(interval, 0, &g_timer_id);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_STANDBY_TIMER_ERR_PRINT("mt_unf_timer_request failed, ret = %d\n", Ret);
        return Ret;
    }

    /** 启动信号驱动机制 */
    signal(SIGIO, MT_StandbyTimerModeTimerDefaultCallback);

    /*
    F_SETOWN：设置将要在文件描述词fd上接收SIGIO
    F_SETFL ：设置文件状态标志
    F_GETFL ：读取文件状态标志。
    FASYNC:   启用异步通知机制
    */
    fcntl(g_s32TimerDevFd, F_SETOWN, getpid());
    fcntl(g_s32TimerDevFd, F_SETFL, fcntl(g_s32TimerDevFd, F_GETFL) | FASYNC);

    Ret = MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_0, MT_FALSE);
    Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_1, MT_FALSE);
    Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_2, MT_FALSE);
    Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_3, MT_FALSE);
    Ret |= MT_UNF_DISP_SetHdVideoEnable(MT_FALSE);
    Ret |= MT_UNF_DISP_SetSdVideoEnable(MT_FALSE);
    Ret |= MT_UNF_HDMI_Output_Set(MT_UNF_HDMI_ID_0, MT_FALSE);
    Ret |= MT_UNF_HDMI_Stop(MT_UNF_HDMI_ID_0);
    Ret |=MT_UNF_HDMI_Close(MT_UNF_HDMI_ID_0);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_STANDBY_TIMER_ERR_PRINT("Standby failure!\n");
        Ret = mt_unf_timer_release(g_timer_id);
        g_timer_id = (MT_U32)-1;
        if (MT_SUCCESS != Ret)
        {
            SAMPLE_STANDBY_TIMER_ERR_PRINT("mt_unf_timer_release failed, ret = %d\n", Ret);
        }
        return Ret;
    }

    SAMPLE_STANDBY_TIMER_FUNCTION_EXIT();

    return MT_SUCCESS;
}


/*!
@brief False standby wake-up
@param  MT_VOID
@return::MT_SUCCESS             Success.
@return::Ret                    Fail.
@*/
static MT_S32 MT_StandbyTimerModeWakeup(MT_VOID)
{
    MT_S32                   Ret = MT_FAILURE;
    MT_UNF_HDMI_OPEN_PARA_S  DefaultMode = { 0 };

    SAMPLE_STANDBY_TIMER_FUNCTION_ENTER();

    Ret = MT_UNF_HDMI_Open(MT_UNF_HDMI_ID_0, &DefaultMode);
    Ret |= MT_UNF_HDMI_Start(MT_UNF_HDMI_ID_0);
    Ret |= MT_UNF_HDMI_Output_Set(MT_UNF_HDMI_ID_0, MT_TRUE);
    Ret |= MT_UNF_DISP_SetSdVideoEnable(MT_TRUE);
    Ret |= MT_UNF_DISP_SetHdVideoEnable(MT_TRUE);
    Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_3, MT_TRUE);
    Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_2, MT_TRUE);
    Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_1, MT_TRUE);
    Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_0, MT_TRUE);
    if(Ret != MT_SUCCESS)
    {
        SAMPLE_STANDBY_TIMER_ERR_PRINT("Wake failed");
        return Ret;
    }

    SAMPLE_STANDBY_TIMER_FUNCTION_EXIT();

    return MT_SUCCESS;
}


static MT_VOID MT_StandbyTimerModePrintMenu(MT_VOID)
{
    MT_STANDBY_TIMER_PRINT("commond: \n");
    MT_STANDBY_TIMER_PRINT(" 1 - n: Standby time(Unit: seconds)\n");
    MT_STANDBY_TIMER_PRINT("     w: Wake \n");
    MT_STANDBY_TIMER_PRINT("     h: help \n");
    MT_STANDBY_TIMER_PRINT("     q: quit \n");
    MT_STANDBY_TIMER_PRINT("STAND>> ");
}


/*!
@brief Enter the standby time or wake up standby
@param  MT_VOID
@return::MT_VOID
@*/
static MT_VOID MT_StandbyTimerModeCmdTask(MT_VOID)
{
    MT_S32  Ret = MT_FAILURE;
    MT_U32  sec = 0;
    MT_CHAR inPut[32] = { 0 };



    while(1)
    {
        (MT_VOID)MT_StandbyTimerModePrintMenu();

        fgets((char *)(inPut), (sizeof(inPut) - 1), stdin);
        MT_STANDBY_TIMER_PRINT("\n");
        if('q' == inPut[0])
        {
            if(g_standby == MT_FALSE)
            {
                break;
            }
            else
            {
                /** Release the timer */
                Ret = mt_unf_timer_release(g_timer_id);
                g_timer_id = (MT_U32)-1;
                if(MT_SUCCESS != Ret)
                {
                    SAMPLE_STANDBY_TIMER_ERR_PRINT("mt_unf_timer_release failed, ret = %d\n", Ret);
                    break;
                }

                Ret = MT_StandbyTimerModeWakeup();
                if(MT_SUCCESS != Ret)
                {
                    SAMPLE_STANDBY_TIMER_ERR_PRINT("MT_StandbyTimerModeWakeup failed, ret = %d\n", Ret);
                    break;
                }

                g_standby = MT_FALSE;

                break;
            }
        }
        else if('w' == inPut[0])
        {
            if(g_standby == MT_FALSE)
            {
                SAMPLE_STANDBY_TIMER_INFO_PRINT("Already awake...\n");
            }
            else
            {
                /** Release the timer */
                Ret = mt_unf_timer_release(g_timer_id);
                g_timer_id = (MT_U32)-1;
                if(MT_SUCCESS != Ret)
                {
                    SAMPLE_STANDBY_TIMER_ERR_PRINT("mt_unf_timer_release failed, ret = %d\n", Ret);
                    break;
                }

                Ret = MT_StandbyTimerModeWakeup();
                if(MT_SUCCESS != Ret)
                {
                    SAMPLE_STANDBY_TIMER_ERR_PRINT("MT_StandbyTimerModeWakeup failed, ret = %d\n", Ret);
                    break;
                }

                g_standby = MT_FALSE;
            }
        }
        else if('h' == inPut[0])
        {
            SAMPLE_STANDBY_TIMER_INFO_PRINT("Print help info \n");
            continue;
        }
        if(atoi(inPut) > 0 )
        {
            sec = atoi(inPut);
            SAMPLE_STANDBY_TIMER_INFO_PRINT("Start standby...\n");

            Ret = MT_StandbyTimerModeSetParam(sec);
            if(MT_SUCCESS != Ret)
            {
                SAMPLE_STANDBY_TIMER_ERR_PRINT("MT_StandbyTimerModeSetParam failed, ret = %d\n", Ret);
                break;
            }
        }
    }
}

#ifdef MT_SAMPLE_APP
MT_S32 MT_StandbyTimerMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif
{
    MT_S32  Ret = MT_FAILURE;

#ifndef MT_SAMPLE_APP
    Ret = mt_sys_init();
    if (MT_SUCCESS != Ret)
    {
        SAMPLE_STANDBY_TIMER_ERR_PRINT("mt_sys_init failed, ret = %d\n", Ret);
        return Ret;
    }

    Ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_STANDBY_TIMER_ERR_PRINT("MTADP_HDMI_Init failed, ret = %d\n", Ret);
        goto ERR0;
    }

    Ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_STANDBY_TIMER_ERR_PRINT("MTADP_Disp_Init failed, ret = %d\n", Ret);
        goto ERR1;
    }
#endif

    Ret = mt_unf_timer_init(&g_s32TimerDevFd);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_STANDBY_TIMER_ERR_PRINT("mt_unf_timer_init failed, ret = %d\n", Ret);
        goto ERR2;
    }

    (MT_VOID)MT_StandbyTimerModeCmdTask();

    (MT_VOID)mt_unf_timer_deinit();
ERR2:
#ifndef MT_SAMPLE_APP
    (MT_VOID)MTADP_Disp_DeInit();
ERR1:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
ERR0:
    (MT_VOID)mt_sys_deinit();
#endif

    return Ret;
}

