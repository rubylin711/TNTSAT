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
#include "mt_adp_mpi.h"
#include "mt_unf_keyled.h"
/***************************** Macro Definition ******************************/
#ifdef MT_STANDBY_FRONT_DEBUG

#define MT_FRONT_PRINT   printf
#else

#define MT_FRONT_PRINT

#endif

#define SAMPLE_FRONT_FUNCTION_ENTER()       MT_FRONT_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_FRONT_FUNCTION_EXIT()        MT_FRONT_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_FRONT_FATAL_PRINT(fmt...)        MT_FRONT_PRINT(" [FATAL] " fmt)
#define SAMPLE_FRONT_ERR_PRINT(fmt...)          MT_FRONT_PRINT(" [ERROR] " fmt)
#define SAMPLE_FRONT_WARN_PRINT(fmt...)         MT_FRONT_PRINT(" [WARN] "  fmt)
#define SAMPLE_FRONT_INFO_PRINT(fmt...)         MT_FRONT_PRINT(" [INFO] "  fmt)
#define SAMPLE_FRONT_DBG_PRINT(fmt...)          MT_FRONT_PRINT(" [DEBUG] " fmt)

#define MT_SAMPLE_FRONT_PRINT  printf
#define MT_TASK_RUN        1
#define MT_TASK_EXIT       2
/*************************** Structure Definition ****************************/
typedef struct keyled
{
    int type;
    int strlen;
    MT_UNF_KEYLED_TYPE_E keyled_type;
}mt_input_keyled;
/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;
static pthread_t g_keyTaskid;
/******************************* API declaration *****************************/

#ifdef MT_SAMPLE_APP
MT_S32 MT_FrontpanelMain(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

/*****************************************************************************
@brief Monitor the status of keys
*param[in] args, void
@return::void
*****************************************************************************/
static MT_VOID * MT_Receive(MT_VOID)
{
    MT_S32 s32Ret;
    mt_u32 u32PressStatus, u32KeyId;

    SAMPLE_FRONT_FUNCTION_ENTER();

    while (g_bTaskQuit == MT_FALSE)
    {
        /*get KEY press value & press status*/
        s32Ret = MT_UNF_KEY_GetValue(&u32PressStatus, &u32KeyId);
        if (MT_SUCCESS == s32Ret)
        {
            SAMPLE_FRONT_INFO_PRINT("KEY  KeyId : 0x%x    PressStatus :%d[%s]\n", u32KeyId, u32PressStatus
                , (0 == u32PressStatus) ? "DOWN"  : (1 == u32PressStatus) ? "HOLD" : "UP");
        }
        else
        {
            MT_USLEEP(50000);
        }

    }
    SAMPLE_FRONT_FUNCTION_EXIT();
    return 0;
}

/*****************************************************************************
 *brief Printthe info of reminding
 *param[in] name: argv[0]
 *return ::void.
*****************************************************************************/

static MT_VOID MT_FrontpanelPrintMenu(MT_VOID)
{
    MT_FRONT_PRINT("commond: \n");
    MT_FRONT_PRINT("     s: Set the digital tube display number \n");
    MT_FRONT_PRINT("     t: Set led light on/off \n");
#ifdef MT_SAMPLE_APP
    MT_FRONT_PRINT("     b: background run \n");
#endif
    MT_FRONT_PRINT("     q: quit \n");
    MT_FRONT_PRINT("     h: help \n");
    MT_FRONT_PRINT("FPANEL>> ");
}


/*****************************************************************************
@brief Gets the value of the key
@return ::void
*****************************************************************************/
static MT_S32 MT_SampleCmdTask(MT_VOID)
{
    MT_CHAR *pfgetret = NULL;
    MT_CHAR inPutCmd[32] ={ 0 };
    MT_U8 data;
    MT_UNF_DIS_PLAY_LBD_E display_lbd;
    MT_BOOL enable = MT_FALSE;
    mt_s32 grid_pos = 0;

    while(1)
    {
        (MT_VOID)MT_FrontpanelPrintMenu();

        pfgetret=fgets((char *)(inPutCmd), (sizeof(inPutCmd) - 1), stdin);
        pfgetret=pfgetret;
        if ('q' == inPutCmd[0])
        {
            g_bTaskQuit = MT_TRUE;
            SAMPLE_FRONT_INFO_PRINT("prepare to exit!\n");
            break;
        }
    #ifdef MT_SAMPLE_APP
        else if ('b' == inPutCmd[0])
        {
            SAMPLE_FRONT_INFO_PRINT("frontpanel play in back!\n");
            break;
        }
    #endif

        else if ('s' == inPutCmd[0])
        {
            SAMPLE_FRONT_INFO_PRINT("Enter the number displayed (Up to four digits) \n");
            scanf("%s",&data);
            getchar();
            MT_UNF_LED_Display_Asc(&data);
        }
        else if ('t' == inPutCmd[0])
        {
            SAMPLE_FRONT_INFO_PRINT("Whether to turn on display (0:off,1:on)\n");
            scanf("%d",&enable);
            getchar();
            SAMPLE_FRONT_INFO_PRINT("Which led([fd650] ex: 1:power led 2:lock led) \n");
            scanf("%d",&grid_pos);
            getchar();
            display_lbd.enable = enable;//0:power off, 1:power on
            display_lbd.grid_pos = grid_pos;//which led
            display_lbd.seg_pos = 7;//which segment
            MT_UNF_LED_Display_LBD(&display_lbd);
        }

        else if ('h' == inPutCmd[0])
        {
            SAMPLE_FRONT_INFO_PRINT("Print help info \n");
        }
    }
    return 0;
}


static MT_VOID MT_FrontpanelExit(MT_VOID)
{
    SAMPLE_FRONT_FUNCTION_ENTER();

    (MT_VOID)MT_UNF_LED_Close();

    (MT_VOID)MT_UNF_KEYLED_DeInit();

    g_bTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_keyTaskid, 0);

    memset(&g_keyTaskid, 0xff, sizeof(g_keyTaskid));

    SAMPLE_FRONT_FUNCTION_EXIT();
}


static MT_VOID MT_FrontpanelPrint_Help(MT_CHAR *name)
{
    MT_FRONT_PRINT("Lack of parameters\n");
    MT_FRONT_PRINT("\nUsage:\n");
    MT_FRONT_PRINT("%s\n", name);
    MT_FRONT_PRINT("    -t:   Front panel type, 1: fd650, 2: CT1642\n");
#ifdef MT_SAMPLE_APP
    MT_FRONT_PRINT("    -q:   Exit the background\n");
#endif
    MT_FRONT_PRINT("example:\n");
    MT_FRONT_PRINT("    %s -t 0\n", name);
}

static MT_S32 MT_FrontpanelParase_args(MT_S32 argc, MT_CHAR *argv[], MT_UNF_KEYLED_TYPE_E *keyled_type)
{
    MT_S32 opt = 0;

    SAMPLE_FRONT_FUNCTION_ENTER();

    if(argc != 3 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_FrontpanelPrint_Help(argv[0]);
        return MT_FAILURE;
    }

    while((opt = MTADP_Getopt(argc, argv, ":?hHt:q")) != -1)
    {
        switch(opt)
        {
            case '?':
            case 'h':
            case 'H':
                (MT_VOID)MT_FrontpanelPrint_Help(argv[0]);
                return MT_FAILURE;
            case 't':
                *keyled_type = strtol(mt_optarg, 0, 0);
                break;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_FrontpanelExit();
                }
                return MT_TASK_EXIT;
            default:
                (MT_VOID)MT_FrontpanelPrint_Help(argv[0]);
                return MT_FAILURE;
        }
    }

    SAMPLE_FRONT_FUNCTION_EXIT();

    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_FrontpanelMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    MT_S32 s32Ret = 0;
    MT_UNF_KEYLED_TYPE_E keyled = { 0 };

    SAMPLE_FRONT_INFO_PRINT("Show led and wait key press\n");

    s32Ret = MT_FrontpanelParase_args(argc, argv, &keyled);
    if (MT_FAILURE == s32Ret)
    {
        SAMPLE_FRONT_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == s32Ret)
    {
        SAMPLE_FRONT_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP
        s32Ret = mt_sys_init();
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_FRONT_ERR_PRINT("mt_sys_init failed, ret = %d\n", s32Ret);
            goto ERR0;
        }
#endif
        s32Ret = MT_UNF_KEYLED_Init();
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_FRONT_ERR_PRINT("MT_UNF_KEYLED_Init failed, ret = %d\n", s32Ret);
            goto ERR0;
        }

        /* open LED device */
        s32Ret = MT_UNF_LED_Open();
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_FRONT_ERR_PRINT("ErrorCode=0x%x\n",s32Ret);
            goto ERR1;
        }

        SAMPLE_FRONT_INFO_PRINT("MT_UNF_KEYLED_Open end!\n");

        s32Ret = MT_UNF_KEYLED_SelectType(keyled);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_FRONT_ERR_PRINT("ErrorCode=0x%x\n",s32Ret);
            goto ERR1;
        }

        s32Ret = MT_UNF_KEY_RepKeyTimeoutVal(200);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_FRONT_ERR_PRINT("ErrorCode=0x%x\n",s32Ret);
            goto ERR2;
        }

        s32Ret = MT_UNF_KEY_IsRepKey(1);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_FRONT_ERR_PRINT("ErrorCode=0x%x\n",s32Ret);
            goto ERR2;
        }

        s32Ret = MT_UNF_KEY_IsKeyUp(0);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_FRONT_ERR_PRINT("ErrorCode=0x%x\n",s32Ret);
            goto ERR2;
        }

        s32Ret = MT_UNF_LED_Display(0x3f065b4f); //0123
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_FRONT_ERR_PRINT("MT_UNF_LED_Display=0x%x\n",s32Ret);
            goto ERR2;
        }

        g_bTaskQuit = MT_FALSE;
        /* create a thread for receive */
        s32Ret = pthread_create(&g_keyTaskid, NULL, (MT_VOID * (*)(MT_VOID *))MT_Receive, NULL);
        if (0 != s32Ret)
        {
            SAMPLE_FRONT_ERR_PRINT("ErrorCode=0x%x\n", s32Ret);
            goto ERR2;
        }
    }

    (MT_VOID)MT_SampleCmdTask();

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

    g_bTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_keyTaskid, 0);
ERR2:
    (MT_VOID)MT_UNF_LED_Close();
ERR1:
    (MT_VOID)MT_UNF_KEYLED_DeInit();
ERR0:
#ifndef MT_SAMPLE_APP
    (MT_VOID)mt_sys_deinit();
#endif

    return s32Ret;
}



