#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mt_unf_ir.h>
#include <mt_unf_keyled.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "mt_unf_pm.h"
#include "mt_unf_ir.h"
#include "mt_unf_hdmi.h"
#include "mt_unf_disp.h"
#include "mt_adp_hdmi.h"
#include "mt_adp_mpi.h"



/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_TIMER_DEBUG

#define MT_TIMER_PRINT   printf
#else

#define MT_TIMER_PRINT

#endif

#define SAMPLE_TIMER_FUNCTION_ENTER()       MT_TIMER_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_TIMER_FUNCTION_EXIT()        MT_TIMER_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_TIMER_FATAL_PRINT(fmt...)    MT_TIMER_PRINT(" [FATAL] " fmt)
#define SAMPLE_TIMER_ERR_PRINT(fmt...)      MT_TIMER_PRINT(" [ERROR] " fmt)
#define SAMPLE_TIMER_WARN_PRINT(fmt...)     MT_TIMER_PRINT(" [WARN] "  fmt)
#define SAMPLE_TIMER_INFO_PRINT(fmt...)     MT_TIMER_PRINT(" [INFO] "  fmt)
#define SAMPLE_TIMER_DBG_PRINT(fmt...)      MT_TIMER_PRINT(" [DEBUG] " fmt)

#define SAMPLE_TIMER_PRINT printf

#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2

static MT_BOOL g_bTaskQuit = MT_TRUE;

#ifdef MT_SAMPLE_APP
MT_S32 MT_PowerTimerMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

/*
@brief True standby function, set the timing start time
@param[in] Sec, Timing start time
@return MT_SUCCESS
@return MT_FAILURE
*/
static void MT_PowerTimer_standby( MT_S32  Sec)
{
    mt_s32 s32Ret = 0;
    sty_wakeup_conf_t wconfig = { 0 };

    s32Ret = MT_UNF_PMOC_Init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_TIMER_ERR_PRINT("MT_UNF_PMOC_Init failed\n");
    }

    s32Ret = MT_UNF_PMOC_SetDevType(3);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_TIMER_ERR_PRINT("MT_UNF_PMOC_SetDevType failed\n");
    }

    SAMPLE_TIMER_INFO_PRINT("MT_UNF_PMOC_SetDevType 3..\n");
    SAMPLE_TIMER_INFO_PRINT("set weak up time \n");

    SAMPLE_TIMER_INFO_PRINT("MT_UNF_PMOC_SetGpenPin 0..\n");
    s32Ret = MT_UNF_PMOC_SetGpenPin(0);	
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_TIMER_ERR_PRINT("MT_UNF_PMOC_SetGpenPin  failed\n");
    }

    wconfig.w_mode = TIME_WAKE_UP;

    wconfig.w_time.wakeup_sec = Sec;
    /* The standby time is min,hour,day to support
    wconfig.w_time.wakeup_min = Min;
    wconfig.w_time.wakeup_hour = time1;
    wconfig.w_time.pass_day = time;
    */

    s32Ret = MT_UNF_PMOC_SetWakeUpAttr(wconfig);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_TIMER_ERR_PRINT("MT_UNF_PMOC_SetWakeUpAttr  failed\n");
    }

	s32Ret = MTADP_Switch_Standby_Mode();
	if(MT_SUCCESS != s32Ret)
	{
		SAMPLE_TIMER_ERR_PRINT("MTADP_Switch_Standby_Mode  failed\n");
	}

    SAMPLE_TIMER_INFO_PRINT(" start low power......\n");
    s32Ret = MT_UNF_PMOC_SwitchSystemMode();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_TIMER_ERR_PRINT("MT_UNF_PMOC_SwitchSystemMode  failed\n");
    }


}
static MT_VOID MT_PowerTimerPrintMenu(void)
{
    SAMPLE_TIMER_PRINT("1-59(sec) to enter standby \n");
#ifdef MT_SAMPLE_APP
    SAMPLE_TIMER_PRINT("     b : background run \n");
#endif
    SAMPLE_TIMER_PRINT("     h : help \n");
    SAMPLE_TIMER_PRINT("     q : quit \n");
    SAMPLE_TIMER_PRINT("Timer>> ");

}

/*
@brief quit and Set time
@return void
*/
static void MT_PowerTimer_CmdTask(void)
{
    MT_S32  sec = 0;
    MT_CHAR inPut[32] = { 0 };

    while(1)
    {
        (MT_VOID)MT_PowerTimerPrintMenu();
        fgets((char *)(inPut), (sizeof(inPut) - 1), stdin);
        if('q' == inPut[0])
        {
            SAMPLE_TIMER_INFO_PRINT("exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
        #ifdef MT_SAMPLE_APP
        else if('b' == inPut[0])
        {
            SAMPLE_TIMER_INFO_PRINT("power_timer in back!\n");
            break;
        }
        #endif
        sec = atoi(inPut);
        if(sec > 0 && sec < 60)
        {
            SAMPLE_TIMER_INFO_PRINT("Start standby...\n");
            (void)MT_PowerTimer_standby(sec);
        }
        else
        {
            SAMPLE_TIMER_INFO_PRINT("please enter sec is Less than 60s\n");
        }


    }
}

static mt_s32 MT_PowerTimerParase_args(int argc, char *argv[])
{
    int opt = 0;


    SAMPLE_TIMER_FUNCTION_ENTER();

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
                    g_bTaskQuit = MT_TRUE;
                }
                return MT_TASK_EXIT;


            default:
                return MT_FAILURE;
            break;
        }
    }
    SAMPLE_TIMER_FUNCTION_EXIT();
    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_PowerTimerMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    MT_S32  Ret = MT_FAILURE;

    if(argc != 1 && g_bTaskQuit == MT_TRUE)
    {
        return MT_SUCCESS;
    }
    Ret = MT_PowerTimerParase_args(argc, argv);
    if (MT_FAILURE == Ret)
    {
        SAMPLE_TIMER_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == Ret)
    {
        SAMPLE_TIMER_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }
    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP
        Ret = mt_sys_init();
        if (MT_SUCCESS != Ret)
        {
            SAMPLE_TIMER_ERR_PRINT("mt_sys_init failed, ret = %d\n", Ret);
            return Ret;
        }

        Ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_TIMER_ERR_PRINT("MTADP_HDMI_Init failed, ret = %d\n", Ret);
            goto ERR1;
        }

        Ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_TIMER_ERR_PRINT("MTADP_Disp_Init failed, ret = %d", Ret);
            goto ERR2;
        }

#endif
        g_bTaskQuit = MT_FALSE;
    }

    (void)MT_PowerTimer_CmdTask();
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

    return Ret;
}

