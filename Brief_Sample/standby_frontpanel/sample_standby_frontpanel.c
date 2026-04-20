/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mt_unf_keyled.h"
#include "mt_unf_hdmi.h"
#include "mt_adp_hdmi.h"
#include "mt_unf_disp.h"
#include "mt_adp_mpi.h"

/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_STANDBY_FRONT_DEBUG

#define MT_STANDBY_FRONT_PRINT   printf
#else

#define MT_STANDBY_FRONT_PRINT

#endif

#define SAMPLE_STANDBY_FRONT_FUNCTION_ENTER()       MT_STANDBY_FRONT_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_STANDBY_FRONT_FUNCTION_EXIT()        MT_STANDBY_FRONT_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_STANDBY_FRONT_FATAL_PRINT(fmt...)        MT_STANDBY_FRONT_PRINT(" [FATAL] " fmt)
#define SAMPLE_STANDBY_FRONT_ERR_PRINT(fmt...)          MT_STANDBY_FRONT_PRINT(" [ERROR] " fmt)
#define SAMPLE_STANDBY_FRONT_WARN_PRINT(fmt...)         MT_STANDBY_FRONT_PRINT(" [WARN] "  fmt)
#define SAMPLE_STANDBY_FRONT_INFO_PRINT(fmt...)         MT_STANDBY_FRONT_PRINT(" [INFO] "  fmt)
#define SAMPLE_STANDBY_FRONT_DBG_PRINT(fmt...)          MT_STANDBY_FRONT_PRINT(" [DEBUG] " fmt)

#define MT_TASK_RUN        1
#define MT_TASK_EXIT       2
/*************************** Structure Definition ****************************/
/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;
static pthread_t g_keyTaskid;
/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_StandbyFrontpanelMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

/*****************************************************************************
*brief Enter false standby
*param  void
*return::MT_SUCCESS             Success.
*return::MT_FAILURE             Fail.
*****************************************************************************/
static MT_S32 MT_Fake_Standby(void)
{
    MT_S32 Ret = MT_FAILURE;

    SAMPLE_STANDBY_FRONT_FUNCTION_ENTER();

    Ret = MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_0, MT_FALSE);
    Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_1, MT_FALSE);
    Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_2, MT_FALSE);
    Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_3, MT_FALSE);
    Ret |= MT_UNF_DISP_SetHdVideoEnable(MT_FALSE);
    Ret |= MT_UNF_DISP_SetSdVideoEnable(MT_FALSE);
    Ret |= MT_UNF_HDMI_Output_Set(MT_UNF_HDMI_ID_0, MT_FALSE);
    Ret |= MT_UNF_HDMI_Stop(MT_UNF_HDMI_ID_0);
    Ret |=MT_UNF_HDMI_Close(MT_UNF_HDMI_ID_0);

    if (MT_SUCCESS != Ret)
    {

    SAMPLE_STANDBY_FRONT_ERR_PRINT("errret Ret ==%d\n",Ret);
        return MT_FAILURE;
    }

    SAMPLE_STANDBY_FRONT_FUNCTION_EXIT();
    return MT_SUCCESS;
}

/*****************************************************************************
*brief False standby wake-up
*param  void
*return::MT_SUCCESS             Success.
*return::MT_FAILURE             Fail.
*****************************************************************************/
static MT_S32 MT_Fake_Wakeup(void)
{
    MT_S32                   Ret = MT_FAILURE;
    MT_UNF_HDMI_OPEN_PARA_S  DefaultMode = { 0 };

    SAMPLE_STANDBY_FRONT_FUNCTION_ENTER();


    Ret = MT_UNF_HDMI_Open(MT_UNF_HDMI_ID_0, &DefaultMode);
    Ret |= MT_UNF_HDMI_Start(MT_UNF_HDMI_ID_0);
    Ret |= MT_UNF_HDMI_Output_Set(MT_UNF_HDMI_ID_0, MT_TRUE);
    Ret |= MT_UNF_DISP_SetSdVideoEnable(MT_TRUE);
    Ret |= MT_UNF_DISP_SetHdVideoEnable(MT_TRUE);
    Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_3, MT_TRUE);
    Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_2, MT_TRUE);
    Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_1, MT_TRUE);
    Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_0, MT_TRUE);
    if (MT_SUCCESS != Ret)
    {
        SAMPLE_STANDBY_FRONT_ERR_PRINT("errret Ret ==%d\n",Ret);
        return MT_FAILURE;
    }

    SAMPLE_STANDBY_FRONT_FUNCTION_EXIT();

  return MT_SUCCESS;
}



/*****************************************************************************
*brief Monitor the status of keys
*param[in] args, void
*return::void
*****************************************************************************/
static MT_VOID *MT_Receive(MT_VOID)
{
    MT_S32 Ret = MT_FAILURE;
    MT_U32 u32PressStatus = 0;
    MT_U32 u32KeyId = 0;
    MT_BOOL is_standby = MT_FALSE;
    time_t         currentTime = time(NULL);
    MT_UNF_KEYLED_TIME_S timer;
    struct tm *localTime = localtime(&currentTime);

    MT_STANDBY_FRONT_PRINT("\nKey menu:\n"
                        " Key[0] is standby or wake-up\n"
                        "The other keys are invalid\n");

    while(g_bTaskQuit == MT_FALSE)
    {
        /* get KEY press value & press status */
        Ret = MT_UNF_KEY_GetValue(&u32PressStatus, &u32KeyId);
        if(MT_SUCCESS == Ret)
        {
            if(0x5f == u32KeyId)
            {
                if(MT_FALSE == is_standby)
                {
                    SAMPLE_STANDBY_FRONT_INFO_PRINT("Start standby...\n");
                    Ret = MT_Fake_Standby();
                    if(MT_SUCCESS != Ret)
                    {
                        SAMPLE_STANDBY_FRONT_ERR_PRINT("MT_Fake_Standby failed, ret = %d\n", Ret);
                        g_bTaskQuit = MT_TRUE;
                        break;
                    }
                    is_standby = MT_TRUE;
                    SAMPLE_STANDBY_FRONT_INFO_PRINT("Successful standby!\n");
                }
                else if(MT_TRUE == is_standby)
                {
                    SAMPLE_STANDBY_FRONT_INFO_PRINT("Start waking up...\n");
                    Ret = MT_Fake_Wakeup();
                    sleep(2);
                    if(MT_SUCCESS != Ret)
                    {
                        SAMPLE_STANDBY_FRONT_ERR_PRINT("Wake failed!\n");
                        g_bTaskQuit = MT_TRUE;
                        break;
                    }
                    is_standby = MT_FALSE;
                    SAMPLE_STANDBY_FRONT_INFO_PRINT("Successful wake-up!\n");
                }
            }
            else
            {
                MT_STANDBY_FRONT_PRINT("\nKey menu:\n"
                        " Key[0] is standby or wake-up\n"
                        "The other keys are invalid\n");
            }
        }
        else
        {
            MT_USLEEP(50000);
        }

        currentTime = time(NULL);
        localTime = localtime(&currentTime);

        timer.u32Hour = localTime->tm_hour;
        timer.u32Minute = localTime->tm_min;
        MT_UNF_LED_DisplayTime(timer);
    }

    return 0;
}


static MT_VOID MT_StandbyFrontpanelExit(MT_VOID)
{
    SAMPLE_STANDBY_FRONT_FUNCTION_ENTER();

    (MT_VOID)MT_UNF_LED_Close();

    (MT_VOID)MT_UNF_KEYLED_DeInit();

    g_bTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_keyTaskid, 0);

    memset(&g_keyTaskid, 0xff, sizeof(g_keyTaskid));

    SAMPLE_STANDBY_FRONT_FUNCTION_EXIT();
}


static MT_VOID MT_StandbyFrontpanelPrintMenu(MT_VOID)
{
    MT_STANDBY_FRONT_PRINT("commond: \n");
#ifdef MT_SAMPLE_APP
    MT_STANDBY_FRONT_PRINT("     b: background run \n");
#endif
    MT_STANDBY_FRONT_PRINT("     q: quit \n");
    MT_STANDBY_FRONT_PRINT("     h: help \n");
    MT_STANDBY_FRONT_PRINT("STANDBYFP>> ");
}


static mt_s32 MT_StandbyFrontpanelCmdTask(MT_VOID)
{
    MT_CHAR *pfgetret = NULL;
    MT_CHAR inPutCmd[32] ={ 0 };

    while(1)
    {
        (MT_VOID)MT_StandbyFrontpanelPrintMenu();
        pfgetret=fgets((char *)(inPutCmd), (sizeof(inPutCmd) - 1), stdin);
        pfgetret=pfgetret;
        if ('q' == inPutCmd[0])
        {
            g_bTaskQuit = MT_TRUE;
            SAMPLE_STANDBY_FRONT_INFO_PRINT("prepare to exit!\n");
            break;
        }
    #ifdef MT_SAMPLE_APP
        else if ('b' == inPutCmd[0])
        {
            SAMPLE_STANDBY_FRONT_INFO_PRINT("standby frontpanel play in back!\n");
            break;
        }
    #endif
        else if ('h' == inPutCmd[0])
        {
            SAMPLE_STANDBY_FRONT_INFO_PRINT("Print help info \n");
        }
    }

    return 0;
}


static MT_VOID MT_StandbyFrontpanelPrint_Help(MT_CHAR *name)
{
    MT_STANDBY_FRONT_PRINT("Lack of parameters\n");
    MT_STANDBY_FRONT_PRINT("\nUsage:\n");
    MT_STANDBY_FRONT_PRINT("%s\n", name);
    MT_STANDBY_FRONT_PRINT("    -t:   Front panel type, 0: fd650, 1: CT1642\n");
#ifdef MT_SAMPLE_APP
    MT_STANDBY_FRONT_PRINT("    -q:   Exit the background\n");
#endif
    MT_STANDBY_FRONT_PRINT("example:\n");
    MT_STANDBY_FRONT_PRINT("    %s -t 0\n", name);
}

static MT_S32 MT_StandbyFrontpanelParase_args(MT_S32 argc, MT_CHAR *argv[], MT_UNF_KEYLED_TYPE_E *keyled_type)
{
    MT_S32 opt = 0;

    SAMPLE_STANDBY_FRONT_FUNCTION_ENTER();

    if(argc != 3 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_StandbyFrontpanelPrint_Help(argv[0]);
        return MT_FAILURE;
    }

    while((opt = MTADP_Getopt(argc, argv, ":?hHt:q")) != -1)
    {
        switch(opt)
        {
            case '?':
            case 'h':
            case 'H':
                (MT_VOID)MT_StandbyFrontpanelPrint_Help(argv[0]);
                return MT_FAILURE;
            case 't':
                *keyled_type = strtol(mt_optarg, 0, 0);
                break;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_StandbyFrontpanelExit();
                }
                return MT_TASK_EXIT;
            default:
                (MT_VOID)MT_StandbyFrontpanelPrint_Help(argv[0]);
                return MT_FAILURE;
        }
    }

    SAMPLE_STANDBY_FRONT_FUNCTION_EXIT();

    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_StandbyFrontpanelMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif
{
    mt_s32 s32Ret = 0;
    MT_UNF_KEYLED_TYPE_E keyled_type = { 0 };

    s32Ret = MT_StandbyFrontpanelParase_args(argc,argv, &keyled_type);
    if (MT_FAILURE == s32Ret)
    {
        SAMPLE_STANDBY_FRONT_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == s32Ret)
    {
        SAMPLE_STANDBY_FRONT_INFO_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP
        s32Ret = mt_sys_init();
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_STANDBY_FRONT_ERR_PRINT("mt_sys_init failed, ret = %d\n", s32Ret);
            return s32Ret;
        }

        s32Ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_STANDBY_FRONT_ERR_PRINT("MTADP_HDMI_Init failed, ret = %d\n", s32Ret);
            goto ERR0;
        }

        s32Ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_STANDBY_FRONT_ERR_PRINT("MTADP_Disp_Init failed, ret = %d\n", s32Ret);
            goto ERR1;
        }
#endif
        g_bTaskQuit = MT_FALSE;

        s32Ret = MT_UNF_KEYLED_Init();
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_STANDBY_FRONT_ERR_PRINT("MT_UNF_KEYLED_Init failed, ret = %d\n", s32Ret);
            goto ERR2;
        }

        SAMPLE_STANDBY_FRONT_INFO_PRINT("init ok1\n");
        /* open LED device */
        s32Ret = MT_UNF_LED_Open();
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_STANDBY_FRONT_ERR_PRINT("MT_UNF_LED_Open failed, ret = %d\n", s32Ret);
            goto ERR3;
        }

        s32Ret = MT_UNF_KEYLED_SelectType(keyled_type);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_STANDBY_FRONT_ERR_PRINT("MT_UNF_KEYLED_SelectType failed, ret = %d\n", s32Ret);
            goto ERR4;
        }

        s32Ret = MT_UNF_KEY_RepKeyTimeoutVal(200);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_STANDBY_FRONT_ERR_PRINT("MT_UNF_KEY_RepKeyTimeoutVal failed, ret = %d\n", s32Ret);
            goto ERR4;
        }

        s32Ret = MT_UNF_KEY_IsRepKey(1);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_STANDBY_FRONT_ERR_PRINT("MT_UNF_KEY_IsRepKey failed, ret = %d\n", s32Ret);
            goto ERR4;
        }

        s32Ret = MT_UNF_KEY_IsKeyUp(0);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_STANDBY_FRONT_ERR_PRINT("MT_UNF_KEY_IsKeyUp failed, ret = %d\n", s32Ret);
            goto ERR4;
        }

        s32Ret = pthread_create(&g_keyTaskid, NULL, (MT_VOID * (*)(MT_VOID *))MT_Receive, NULL);
        if (0 != s32Ret)
        {
            SAMPLE_STANDBY_FRONT_ERR_PRINT("ErrorCode=0x%x\n", s32Ret);
            goto ERR4;
        }
    }

    (MT_VOID)MT_StandbyFrontpanelCmdTask();

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

    g_bTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_keyTaskid, 0);

    s32Ret = MT_UNF_LED_Display(0); //on
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STANDBY_FRONT_ERR_PRINT("MT_UNF_LED_Display failed, ret = %d\n", s32Ret);
    }
ERR4:
    (MT_VOID)MT_UNF_LED_Close();
ERR3:
    (MT_VOID)MT_UNF_KEYLED_DeInit();
ERR2:
#ifndef MT_SAMPLE_APP
    (MT_VOID)MTADP_Disp_DeInit();
ERR1:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
ERR0:
    (MT_VOID)mt_sys_deinit();
#endif

    return s32Ret;
}



