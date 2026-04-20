/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "mt_unf_pm.h"
#include "mt_adp_mpi.h"
#include "mt_adp_hdmi.h"
#include "mt_adp_frontend.h"
#include "mt_adp_config.h"
#include "mt_cmdline.h"
/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_STR_TIMER_DEBUG

#define MT_STR_TIMER_PRINT   printf
#else

#define MT_STR_TIMER_PRINT

#endif

#define SAMPLE_STR_TIMER_FUNCTION_ENTER()            MT_STR_TIMER_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_STR_TIMER_FUNCTION_EXIT()             MT_STR_TIMER_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_STR_TIMER_FATAL_PRINT(fmt...)          MT_STR_TIMER_PRINT(" [FATAL] " fmt)
#define SAMPLE_STR_TIMER_ERR_PRINT(fmt...)            MT_STR_TIMER_PRINT(" [ERROR] " fmt)
#define SAMPLE_STR_TIMER_WARN_PRINT(fmt...)           MT_STR_TIMER_PRINT(" [WARN] "  fmt)
#define SAMPLE_STR_TIMER_INFO_PRINT(fmt...)           MT_STR_TIMER_PRINT(" [INFO] "  fmt)
#define SAMPLE_STR_TIMER_DBG_PRINT(fmt...)            MT_STR_TIMER_PRINT(" [DEBUG] " fmt)

#define SAMPLE_STR_TIMER_PRINT printf

#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2
/*************************** Structure Definition ****************************/


/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;
/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_StrTimer_standbyMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

/*****************************************************************************
*brief Enter Real  standby
*param  void
*return::MT_SUCCESS             Success.
*return::MT_FAILURE             Fail.
*****************************************************************************/
static mt_s32 MT_StrTimerOpenModule(void)
{
    mt_s32 s32Ret = MT_SUCCESS;
    config_t config = { 0 };

    s32Ret = mt_sys_init();
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_TIMER_ERR_PRINT("mt_sys_init error. ret=0x%x \n", s32Ret);
        return s32Ret;
    }

    s32Ret = MTADP_Read_All_Config(&config);
    if((MT_SUCCESS != s32Ret) || (MT_TRUE != config.is_use))
    {
        SAMPLE_STR_TIMER_ERR_PRINT("MTADP_Flash_Read failed, ret = %x\n", s32Ret);
        MTADP_Config_Set_Default(&config);
    }

    (mt_void)MTADP_HDMI_Set_HdcpEnable(config.hdcp);

    s32Ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, config.disp_fmt.format);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_TIMER_ERR_PRINT("MTADP_HDMI_Init failed, ret = %x\n", s32Ret);
        goto ERR0;
    }

    /** Display initialization */
    s32Ret = MTADP_Disp_Init(config.disp_fmt.format);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_TIMER_ERR_PRINT("MTADP_Disp_Init failed, ret = %x\n", s32Ret);
        goto ERR1;
    }

    return s32Ret;

ERR1:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
ERR0:
    (MT_VOID)mt_sys_deinit();

    return s32Ret;
}

static mt_s32 MT_StrTimerCloseModule(void)
{
    (MT_VOID)MTADP_Disp_DeInit();

    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);

    (MT_VOID)mt_sys_deinit();

    return MT_SUCCESS;
}

static void MT_StrTimerStartStandby(sty_wakeup_conf_t wconfig)
{
    mt_s32 s32Ret = MT_SUCCESS;

    SAMPLE_STR_TIMER_INFO_PRINT("Low power test for str\n");

    s32Ret = MT_UNF_PMOC_Init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_TIMER_ERR_PRINT("MT_UNF_PMOC_Init failed\n");
        return;
    }

    s32Ret = MT_UNF_PMOC_SetWakeUpAttr(wconfig);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_TIMER_ERR_PRINT("MT_UNF_PMOC_SetWakeUpAttr  failed\n");
    }

    /*First close hdmi/disp/sys module, otherwise can not enter str suspend.*/
    (MT_VOID)MT_StrTimerCloseModule();

    /*Check whether the udhcpc process exists, and kill it if it does.*/
    s32Ret = system("pidof udhcpc");
    if(MT_SUCCESS == s32Ret)
    {
        s32Ret = system("kill $(pidof udhcpc)");
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_STR_TIMER_ERR_PRINT("kill udhcpc failed\n");
        }
    }

    s32Ret = system("killall -SIGUSR1 audio_ta_service");
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_TIMER_ERR_PRINT("killall -SIGUSR1 audio_ta_service failed\n");
    }

    s32Ret = system("echo userspace > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor");
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_TIMER_ERR_PRINT("echo userspace > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor failed\n");
    }

    mt_msleep(200);
#ifdef CONFIG_MT_CHIP_SYMPHONY6
    s32Ret = system("echo 960000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed");
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_TIMER_ERR_PRINT("echo 960000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed failed\n");
    }
#else
    //symphony4
    s32Ret = system("echo 720000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed");
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_TIMER_ERR_PRINT("echo 720000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed failed\n");
    }
#endif

    SAMPLE_STR_TIMER_INFO_PRINT("enter system suspend \n");
    s32Ret = system("echo mem > /sys/power/state");
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_TIMER_ERR_PRINT("echo mem > /sys/power/state failed\n");
    }

    SAMPLE_STR_TIMER_INFO_PRINT("exit system suspend \n");
    s32Ret = system("optee_load_avfw -l 1 -f /usr/local/stb/avfw/avfw.bin");
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_TIMER_ERR_PRINT("optee_load_avfw -l 1 -f /usr/local/stb/avfw/avfw.bin failed\n");
    }

    s32Ret = system("killall -SIGUSR2 audio_ta_service");
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_TIMER_ERR_PRINT("killall -SIGUSR2 audio_ta_service failed\n");
    }

    s32Ret = system("echo performance > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor");
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_TIMER_ERR_PRINT("echo performance > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor failed\n");
    }

    s32Ret = MT_StrTimerOpenModule();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_TIMER_ERR_PRINT("call MT_StrTimerOpenModule failed\n");
    }

    s32Ret = MT_UNF_PMOC_DeInit();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_TIMER_ERR_PRINT("call MT_UNF_PMOC_DeInit failed\n");
    }

    SAMPLE_STR_TIMER_INFO_PRINT("%s: system resume success.\n", __FUNCTION__);
}

static MT_VOID MT_StrTimerPrintMenu(void)
{
    SAMPLE_STR_TIMER_PRINT("     t : input str standby time  \n");

#ifdef MT_SAMPLE_APP
    SAMPLE_STR_TIMER_PRINT("     b : background run \n");
#endif
    SAMPLE_STR_TIMER_PRINT("     h : help \n");
    SAMPLE_STR_TIMER_PRINT("     q : quit \n");
    SAMPLE_STR_TIMER_PRINT("str_timer>> \n");
}

static void MT_StrTimerCmdTask(void)
{
    MT_CHAR inputCmd[32] = { 0 };
    sty_wakeup_conf_t wconfig = {0};
    mt_s32 sec,remaining_sec;

    while (1)
    {
        (MT_VOID)MT_StrTimerPrintMenu();
        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        if ('q' == inputCmd[0])
        {
            SAMPLE_STR_TIMER_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
        else if('t' == inputCmd[0])
        {
            SAMPLE_STR_TIMER_PRINT("\nInput str Standby Time:");
            scanf("%d", &sec);
            getchar();
            wconfig.w_mode = TIME_WAKE_UP;
            wconfig.w_time.pass_day = (MT_U8)(sec / (24*60*60));
            remaining_sec = sec % (24*60*60);
            wconfig.w_time.wakeup_hour = (MT_U8)(remaining_sec / (60*60));
            remaining_sec = remaining_sec % (60*60);
            wconfig.w_time.wakeup_min = (MT_U8)(remaining_sec / 60);
            remaining_sec = remaining_sec % 60;
            wconfig.w_time.wakeup_sec = (MT_U8)remaining_sec;
            wconfig.w_key.fp_wkey = 1;
            SAMPLE_STR_TIMER_INFO_PRINT("wake up time=(%u:%u:%u:%u) key=%d\n", wconfig.w_time.pass_day,
                wconfig.w_time.wakeup_hour,wconfig.w_time.wakeup_min, wconfig.w_time.wakeup_sec, wconfig.w_key.fp_wkey);
            MT_StrTimerStartStandby(wconfig);
        }
#ifdef MT_SAMPLE_APP
        else if('b' == inputCmd[0])
        {
            SAMPLE_STR_TIMER_INFO_PRINT("str in back!\n");
            break;
        }
#endif
        else if('h' == inputCmd[0])
        {
            SAMPLE_STR_TIMER_INFO_PRINT("help info\n");
        }
    }
}
static MT_VOID MT_StrTimer_Exit(MT_VOID)
{
    g_bTaskQuit = MT_TRUE;
}

static mt_s32 MT_StrTimerParase_args(int argc, char *argv[])
{
    int opt = 0;


    SAMPLE_STR_TIMER_FUNCTION_ENTER();

    while((opt = MTADP_Getopt(argc, argv, ":?hHq")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_StrTimer_Exit();
                }
                return MT_TASK_EXIT;


            default:
                return MT_FAILURE;
            break;
        }
    }
    SAMPLE_STR_TIMER_FUNCTION_EXIT();
    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_StrTimer_standbyMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif
{
    MT_S32 ret = MT_FAILURE;

    if(argc != 1 && g_bTaskQuit == MT_TRUE)
    {
        return MT_SUCCESS;
    }

    ret = MT_StrTimerParase_args(argc, argv);
    if (MT_FAILURE == ret)
    {
        SAMPLE_STR_TIMER_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if (MT_TASK_EXIT == ret)
    {
        SAMPLE_STR_TIMER_INFO_PRINT("Recv stop command. stop window.\n");
        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if (MT_SUCCESS != ret)
        {
            SAMPLE_STR_TIMER_ERR_PRINT("mt_sys_init failed, ret = %d\n", ret);
            return ret;
        }

        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_STR_TIMER_ERR_PRINT("MTADP_HDMI_Init failed, ret = %d\n", ret);
            goto ERR1;
        }

        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_STR_TIMER_ERR_PRINT("MTADP_Disp_Init failed, ret = %d", ret);
            goto ERR2;
        }
#endif
        g_bTaskQuit = MT_FALSE;
    }

    (MT_VOID)MT_StrTimerCmdTask();

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

#ifndef MT_SAMPLE_APP
(MT_VOID)MTADP_Disp_DeInit();

ERR2:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
ERR1:
    (MT_VOID)mt_sys_deinit();
#endif

    return ret;
}

