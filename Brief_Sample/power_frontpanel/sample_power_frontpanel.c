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
#include <time.h>
#include "mt_adp_hdmi.h"
#include "mt_adp_mpi.h"
#include "mt_unf_keyled.h"
#include "mt_unf_pm.h"
/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_POWER_DEBUG

#define MT_POWER_PRINT   printf
#else

#define MT_POWER_PRINT
#endif


#define SAMPLE_POWER_FUNCTION_ENTER()           MT_POWER_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_POWER_FUNCTION_EXIT()            MT_POWER_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_POWER_FATAL_PRINT(fmt...)        MT_POWER_PRINT(" [FATAL] " fmt)
#define SAMPLE_POWER_ERR_PRINT(fmt...)          MT_POWER_PRINT(" [ERROR] " fmt)
#define SAMPLE_POWER_WARN_PRINT(fmt...)         MT_POWER_PRINT(" [WARN] "  fmt)
#define SAMPLE_POWER_INFO_PRINT(fmt...)         MT_POWER_PRINT(" [INFO] "  fmt)
#define SAMPLE_POWER_DBG_PRINT(fmt...)          MT_POWER_PRINT(" [DEBUG] " fmt)

#define MT_TASK_RUN        1
#define MT_TASK_EXIT       2
/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;
static pthread_t g_keyTaskid;
/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_PowerFrontpanelMain(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

/*!
@brief Instructions for use
@param  MT_VOID
@return::MT_VOID
@*/
static MT_VOID MT_PowerFrontpanelModePrintMenu(MT_VOID)
{
    MT_POWER_PRINT("\nKey menu:\n");
    MT_POWER_PRINT(" Key[0] is standby\n");
    MT_POWER_PRINT(" Key[1] is wake-up\n");
    MT_POWER_PRINT("The other keys are invalid\n");
}

static MT_VOID *MT_Receive(MT_VOID)
{
    MT_S32         Ret = MT_FAILURE;
    MT_U32         u32PressStatus = 0;
    MT_U32         u32KeyId = 0;
    MT_U32         standbyInfo = 0;
    time_t         currentTime = time(NULL);
    MT_UNF_KEYLED_TIME_S timer;
    struct tm *localTime = localtime(&currentTime);


    standby_time_t getTime = { 0 };

    /** Gets the standby wake state, 0-Normal wake-up, 1-ir wake-up, 2-frontpanel wake-up, 3-timer wake-up */
    Ret = MT_UNF_PMOC_GetStandbyInfo(&standbyInfo);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_POWER_ERR_PRINT("MT_UNF_PMOC_GetStandbyInfo failed, ret = %d\n", Ret);
    }

    if(2 == standbyInfo)
    {
        /** Gets standby time */
        Ret = MT_UNF_PMOC_GetStandbyTime(&getTime);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_POWER_ERR_PRINT("MT_UNF_PMOC_GetStandbyTime failed, ret = %d\n", Ret);
            return NULL;
        }
        SAMPLE_POWER_INFO_PRINT("Get standby time: day:%d hour:%d min:%d sec:%d\n", getTime.pass_day, getTime.cur_hour, getTime.cur_min, getTime.cur_sec);
        SAMPLE_POWER_INFO_PRINT("Wake up successful!\n");
    }
    else
    {
        SAMPLE_POWER_INFO_PRINT("Normal boot successful!\n");
    }

    (MT_VOID)MT_PowerFrontpanelModePrintMenu();

    while (g_bTaskQuit == MT_FALSE)
    {
        /*get KEY press value & press status*/
        Ret = MT_UNF_KEY_GetValue(&u32PressStatus, &u32KeyId);
        if(MT_SUCCESS == Ret)
        {
            if(0x5f == u32KeyId)
            {
                SAMPLE_POWER_INFO_PRINT("Start standby...\n");
                SAMPLE_POWER_INFO_PRINT("Press key[1] to wake up\n");

                /** LED light shows off */
                Ret = MT_UNF_LED_Display(0x3f7171); //off
                if(MT_SUCCESS != Ret)
                {
                    SAMPLE_POWER_ERR_PRINT("MT_UNF_LED_Display failed, ret = %d\n", Ret);
                    break;
                }

                SAMPLE_POWER_INFO_PRINT("MT_UNF_PMOC_SetGpenPin 0..\n");
                Ret = MT_UNF_PMOC_SetGpenPin(0);
                if(MT_SUCCESS != Ret)
                {
                    SAMPLE_POWER_ERR_PRINT("MT_UNF_PMOC_SetGpenPin  failed\n");
                }

                Ret = MTADP_Switch_Standby_Mode();
                if(MT_SUCCESS != Ret)
                {
                    SAMPLE_POWER_ERR_PRINT("MTADP_Switch_Standby_Mode  failed\n");
                }

                /** Enter true standby */
                Ret = MT_UNF_PMOC_SwitchSystemMode();
                if(MT_SUCCESS != Ret)
                {
                    SAMPLE_POWER_ERR_PRINT("MT_UNF_PMOC_SwitchSystemMode failed, ret = %d\n", Ret);
                    break;
                }
            }
            else if(0x57 == u32KeyId)
            {
                SAMPLE_POWER_INFO_PRINT("Already awake\n");
            }
            else
            {
                (MT_VOID)MT_PowerFrontpanelModePrintMenu();
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


static MT_VOID MT_PowerFrontpanelExit(MT_VOID)
{
    SAMPLE_POWER_FUNCTION_ENTER();

    (MT_VOID)MT_UNF_PMOC_DeInit();

    (MT_VOID)MT_UNF_LED_Close();

    (MT_VOID)MT_UNF_KEYLED_DeInit();

    g_bTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_keyTaskid, 0);

    memset(&g_keyTaskid, 0xff, sizeof(g_keyTaskid));

    SAMPLE_POWER_FUNCTION_EXIT();
}


static MT_VOID MT_PowerFrontpanelPrintMenu(MT_VOID)
{
    MT_POWER_PRINT("commond: \n");
#ifdef MT_SAMPLE_APP
    MT_POWER_PRINT("     b: background run \n");
#endif
    MT_POWER_PRINT("     q: quit \n");
    MT_POWER_PRINT("     h: help \n");
    MT_POWER_PRINT("PWFP>> ");
}


/*!
@brief Enter true standby or wake up according to the input key
@param  MT_VOID
@return::MT_VOID
@*/
static MT_VOID MT_PowerFrontpanelModeCmdTask(MT_VOID)
{
    MT_CHAR *pfgetret = NULL;
    MT_CHAR inPutCmd[32] ={ 0 };

    while(1)
    {
        (MT_VOID)MT_PowerFrontpanelPrintMenu();

        pfgetret=fgets((char *)(inPutCmd), (sizeof(inPutCmd) - 1), stdin);
        pfgetret=pfgetret;
        if ('q' == inPutCmd[0])
        {
            g_bTaskQuit = MT_TRUE;
            SAMPLE_POWER_INFO_PRINT("prepare to exit!\n");
            break;
        }
    #ifdef MT_SAMPLE_APP
        else if ('b' == inPutCmd[0])
        {
            SAMPLE_POWER_INFO_PRINT("power frontpanel play in back!\n");
            break;
        }
    #endif
        else if ('h' == inPutCmd[0])
        {
            SAMPLE_POWER_INFO_PRINT("Print help info \n");
        }
    }

    return;
}


static MT_VOID MT_PowerFrontpanelPrint_Help(MT_CHAR *name)
{
    MT_POWER_PRINT("Lack of parameters\n");
    MT_POWER_PRINT("\nUsage:\n");
    MT_POWER_PRINT("%s\n", name);
    MT_POWER_PRINT("    -t:   Front panel type, 1: fd650, 2: CT1642\n");
#ifdef MT_SAMPLE_APP
    MT_POWER_PRINT("    -q:   Exit the background\n");
#endif
    MT_POWER_PRINT("example:\n");
    MT_POWER_PRINT("    %s -t 0\n", name);
}

static MT_S32 MT_PowerFrontpanelParase_args(MT_S32 argc, MT_CHAR *argv[], MT_UNF_KEYLED_TYPE_E *keyled_type)
{
    MT_S32 opt = 0;

    SAMPLE_POWER_FUNCTION_ENTER();

    if(argc != 3 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_PowerFrontpanelPrint_Help(argv[0]);
        return MT_FAILURE;
    }

    while((opt = MTADP_Getopt(argc, argv, ":?hHt:q")) != -1)
    {
        switch(opt)
        {
            case '?':
            case 'h':
            case 'H':
                (MT_VOID)MT_PowerFrontpanelPrint_Help(argv[0]);
                return MT_FAILURE;
            case 't':
                *keyled_type = strtol(mt_optarg, 0, 0);
                break;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_PowerFrontpanelExit();
                }
                return MT_TASK_EXIT;
            default:
                (MT_VOID)MT_PowerFrontpanelPrint_Help(argv[0]);
                return MT_FAILURE;
        }
    }

    SAMPLE_POWER_FUNCTION_EXIT();

    return MT_SUCCESS;
}

static void MT_PowerFrontpanelswitchDevType(MT_UNF_KEYLED_TYPE_E keyled_type, sty_fp_type_t *dev_type)
{
    switch (keyled_type)
    {
        case MT_UNF_KEYLED_TYPE_FD650:
            *dev_type = FD650;
            break;
        case MT_UNF_KEYLED_TYPE_CT1642:
            *dev_type = CT1642;
            break;
        case MT_UNF_KEYLED_TYPE_TT1629B:
            *dev_type = TT1629B;
            break;
        case MT_UNF_KEYLED_TYPE_PT6393:
            *dev_type = PT6393;
            break;
        case MT_UNF_KEYLED_TYPE_KEYADC:
            *dev_type = FD650_KADC;
            break;
        default:
            *dev_type = FD650;
    }

    return;
}

#ifdef MT_SAMPLE_APP
MT_S32 MT_PowerFrontpanelMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    MT_S32               Ret = MT_FAILURE;
    sty_wakeup_conf_t    wakeConfig = { 0 };
    MT_UNF_KEYLED_TYPE_E keyled_type = { 0 };
    sty_fp_type_t dev_type = FD650;

    Ret = MT_PowerFrontpanelParase_args(argc,argv, &keyled_type);
    if (MT_FAILURE == Ret)
    {
        SAMPLE_POWER_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == Ret)
    {
        SAMPLE_POWER_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP
        /** System initialization */
        Ret = mt_sys_init();
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_POWER_ERR_PRINT("MT_SYS_Init failed, ret = %d\n", Ret);
            return Ret;
        }

        /** HDMI initialization */
        Ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_POWER_ERR_PRINT("MTADP_HDMI_Init failed, ret = %d\n", Ret);
            goto ERR0;
        }

        /** Display initialization */
        Ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_POWER_ERR_PRINT("MTADP_Disp_Init failed, ret = %d\n", Ret);
            goto ERR1;
        }
#endif
        g_bTaskQuit = MT_FALSE;
        /** Initialize the front panel module */
        Ret = MT_UNF_KEYLED_Init();
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_POWER_ERR_PRINT("MT_UNF_KEYLED_Init failed, ret = %d\n", Ret);
            goto ERR2;
        }

        /*open LED device*/
        Ret = MT_UNF_LED_Open();
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_POWER_ERR_PRINT("MT_UNF_LED_Open failed, ret = %d\n", Ret);
            goto ERR3;
        }

        /** set frontpanel type */
        Ret = MT_UNF_KEYLED_SelectType(keyled_type);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_POWER_ERR_PRINT("MT_UNF_KEYLED_SelectType failed, ret = %d\n", Ret);
            goto ERR4;
        }

        /** Set the sampling time for repeat keys */
        Ret = MT_UNF_KEY_RepKeyTimeoutVal(200);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_POWER_ERR_PRINT("MT_UNF_KEY_RepKeyTimeoutVal failed, ret = %d\n", Ret);
            goto ERR4;
        }

        /**  Set whether to support duplicate keys */
        Ret = MT_UNF_KEY_IsRepKey(1);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_POWER_ERR_PRINT("MT_UNF_KEY_IsRepKey failed, ret = %d\n", Ret);
            goto ERR4;
        }

        /** Set whether to support key up noticie */
        Ret = MT_UNF_KEY_IsKeyUp(0);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_POWER_ERR_PRINT("MT_UNF_KEY_IsKeyUp failed, ret = %d\n", Ret);
            goto ERR4;
        }

        /** PMOC initialization */
        Ret = MT_UNF_PMOC_Init();
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_POWER_ERR_PRINT("MT_UNF_PMOC_Init failed, ret = %d\n", Ret);
            goto ERR4;
        }

        /** Set the front panel type */
        MT_PowerFrontpanelswitchDevType(keyled_type, &dev_type);
        Ret = MT_UNF_PMOC_SetDevType(dev_type);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_POWER_ERR_PRINT("MT_UNF_PMOC_SetDevType failed, ret = %d\n", Ret);
            goto ERR5;
        }

        /** Set the wake key */
        wakeConfig.w_key.fp_wkey = (mt_u8)0x57;
        Ret = MT_UNF_PMOC_SetWakeUpAttr(wakeConfig);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_POWER_ERR_PRINT("MT_UNF_PMOC_SetWakeUpAttr failed, ret = %d\n", Ret);
            goto ERR5;
        }

        /** Pull the GPEN pin voltage low */
        Ret = MT_UNF_PMOC_SetGpenPin(GPEN_LOW);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_POWER_ERR_PRINT("MT_UNF_PMOC_SetGpenPin failed, ret = %d\n", Ret);
            goto ERR5;
        }

        Ret = pthread_create(&g_keyTaskid, NULL, (MT_VOID * (*)(MT_VOID *))MT_Receive, NULL);
        if(0 != Ret)
        {
            SAMPLE_POWER_ERR_PRINT("ErrorCode=0x%x\n", Ret);
            goto ERR5;
        }
    }

    (MT_VOID)MT_PowerFrontpanelModeCmdTask();

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

    g_bTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_keyTaskid, 0);

    Ret = MT_UNF_LED_Display(0); //on
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_POWER_ERR_PRINT("MT_UNF_LED_Display failed, ret = %d\n", Ret);
    }
ERR5:
    (MT_VOID)MT_UNF_PMOC_DeInit();
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

    return Ret;
}



