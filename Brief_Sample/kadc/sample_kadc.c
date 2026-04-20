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
#ifdef MT_KADC_DEBUG

#define MT_KADC_PRINT   printf
#else

#define MT_KADC_PRINT

#endif

#define SAMPLE_KADC_FUNCTION_ENTER()       MT_KADC_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_KADC_FUNCTION_EXIT()        MT_KADC_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_KADC_FATAL_PRINT(fmt...)    MT_KADC_PRINT(" [FATAL] " fmt)
#define SAMPLE_KADC_ERR_PRINT(fmt...)      MT_KADC_PRINT(" [ERROR] " fmt)
#define SAMPLE_KADC_WARN_PRINT(fmt...)     MT_KADC_PRINT(" [WARN] "  fmt)
#define SAMPLE_KADC_INFO_PRINT(fmt...)     MT_KADC_PRINT(" [INFO] "  fmt)
#define SAMPLE_KADC_DBG_PRINT(fmt...)      MT_KADC_PRINT(" [DEBUG] " fmt)

#define MT_SAMPLE_KADC_PRINT  printf
#define MT_TASK_RUN        1
#define MT_TASK_EXIT       2
/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;
static pthread_t g_keyTaskid;
/******************************* API declaration *****************************/

#ifdef MT_SAMPLE_APP
MT_S32 MT_KadcMain(MT_S32 argc, MT_CHAR *argv[]);
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

    SAMPLE_KADC_FUNCTION_ENTER();

    while (g_bTaskQuit == MT_FALSE)
    {
        /*get KEY press value & press status*/
        s32Ret = MT_UNF_KEY_GetValue(&u32PressStatus, &u32KeyId);
        if (MT_SUCCESS == s32Ret)
        {
            SAMPLE_KADC_INFO_PRINT("KEY  KeyId : 0x%x    PressStatus :%d[%s]\n", u32KeyId, u32PressStatus
                , (0 == u32PressStatus) ? "DOWN"  : (1 == u32PressStatus) ? "HOLD" : "UP");
        }
        else
        {
            MT_USLEEP(50000);
        }

    }
    SAMPLE_KADC_FUNCTION_EXIT();
    return 0;
}

/*****************************************************************************
 *brief Printthe info of reminding
 *param[in] name: argv[0]
 *return ::void.
*****************************************************************************/

static MT_VOID MT_KadcPrintMenu(MT_VOID)
{
    MT_KADC_PRINT("commond: \n");
#ifdef MT_SAMPLE_APP
    MT_KADC_PRINT("     b: background run \n");
#endif
    MT_KADC_PRINT("     q: quit \n");
    MT_KADC_PRINT("     h: help \n");
    MT_KADC_PRINT("KADC>> ");
}


/*****************************************************************************
@brief Gets the value of the key
@return ::void
*****************************************************************************/
static MT_S32 MT_KadcCmdTask(MT_VOID)
{
    MT_CHAR *pfgetret = NULL;
    MT_CHAR inPutCmd[32] ={ 0 };

    while(1)
    {
        (MT_VOID)MT_KadcPrintMenu();

        pfgetret=fgets((char *)(inPutCmd), (sizeof(inPutCmd) - 1), stdin);
        pfgetret=pfgetret;
        if ('q' == inPutCmd[0])
        {
            g_bTaskQuit = MT_TRUE;
            SAMPLE_KADC_INFO_PRINT("Prepare to exit!\n");
            break;
        }
    #ifdef MT_SAMPLE_APP
        else if ('b' == inPutCmd[0])
        {
            SAMPLE_KADC_INFO_PRINT("Kadc play in back!\n");
            break;
        }
    #endif
        else if ('h' == inPutCmd[0])
        {
            SAMPLE_KADC_INFO_PRINT("Print help info \n");
        }
    }
    return 0;
}


static MT_VOID MT_KadcExit(MT_VOID)
{
    SAMPLE_KADC_FUNCTION_ENTER();

    (MT_VOID)MT_UNF_KEYLED_DeInit();

    g_bTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_keyTaskid, 0);

    memset(&g_keyTaskid, 0xff, sizeof(g_keyTaskid));

    SAMPLE_KADC_FUNCTION_EXIT();
}


static MT_VOID MT_KadcPrint_Help(MT_CHAR *name)
{
    MT_KADC_PRINT("Lack of parameters\n");
    MT_KADC_PRINT("\nUsage:\n");
    MT_KADC_PRINT("%s\n", name);
    MT_KADC_PRINT("    -t:   kadc type, 0: 3 keys, 1: 5 keys, 2: 7 keys\n");
#ifdef MT_SAMPLE_APP
    MT_KADC_PRINT("    -q:   Exit the background\n");
#endif
    MT_KADC_PRINT("example:\n");
    MT_KADC_PRINT("    %s -t 2\n", name);
}

static MT_S32 MT_KadcParase_args(MT_S32 argc, MT_CHAR *argv[], MT_UNF_KEYLED_KADC_TYPR_E *keyled_type)
{
    MT_S32 opt = 0;

    SAMPLE_KADC_FUNCTION_ENTER();

    if(argc != 3 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_KadcPrint_Help(argv[0]);
        return MT_FAILURE;
    }

    while((opt = MTADP_Getopt(argc, argv, ":?hHt:q")) != -1)
    {
        switch(opt)
        {
            case '?':
            case 'h':
            case 'H':
                (MT_VOID)MT_KadcPrint_Help(argv[0]);
                return MT_FAILURE;
            case 't':
                *keyled_type = strtol(mt_optarg, 0, 0);
                break;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_KadcExit();
                }
                return MT_TASK_EXIT;
            default:
                (MT_VOID)MT_KadcPrint_Help(argv[0]);
                return MT_FAILURE;
        }
    }

    SAMPLE_KADC_FUNCTION_EXIT();

    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_KadcMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    MT_S32 s32Ret = 0;
    MT_UNF_KEYLED_KADC_TYPR_E kadcType = { 0 };
    MT_UNF_KEYLED_TYPE_V2_E keyled_type_v2 = { 0 };

    s32Ret = MT_KadcParase_args(argc, argv, &kadcType);
    if (MT_FAILURE == s32Ret)
    {
        SAMPLE_KADC_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == s32Ret)
    {
        SAMPLE_KADC_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP
        s32Ret = mt_sys_init();
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_KADC_ERR_PRINT("mt_sys_init failed, ret = 0x%x\n", s32Ret);
            goto ERR0;
        }
#endif
        s32Ret = MT_UNF_KEYLED_Init();
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_KADC_ERR_PRINT("MT_UNF_KEYLED_Init failed, ret = 0x%x\n", s32Ret);
            goto ERR0;
        }

        keyled_type_v2.keyled_type = MT_UNF_KEYLED_TYPE_KEYADC;
        keyled_type_v2.kadc_type   = kadcType;
        s32Ret = MT_UNF_KEYLED_SelectType_V2(keyled_type_v2);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_KADC_ERR_PRINT("MT_UNF_KEYLED_SelectType_V2 failed, ret = 0x%x\n", s32Ret);
            goto ERR1;
        }

        s32Ret = MT_UNF_KEY_setKeyadcType(kadcType);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_KADC_ERR_PRINT("MT_UNF_KEY_IsKeyUp failed, ret = 0x%x\n", s32Ret);
            goto ERR1;
        }

        s32Ret = MT_UNF_KEYLED_Hw_Init();
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_KADC_ERR_PRINT("MT_UNF_KEY_IsKeyUp failed, ret = 0x%x\n", s32Ret);
            goto ERR1;
        }

        s32Ret = MT_UNF_KEY_RepKeyTimeoutVal(200);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_KADC_ERR_PRINT("MT_UNF_KEY_RepKeyTimeoutVal failed, ret = 0x%x\n", s32Ret);
            goto ERR1;
        }

        s32Ret = MT_UNF_KEY_IsRepKey(1);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_KADC_ERR_PRINT("MT_UNF_KEY_IsRepKey failed, ret = 0x%x\n", s32Ret);
            goto ERR1;
        }

        s32Ret = MT_UNF_KEY_IsKeyUp(0);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_KADC_ERR_PRINT("MT_UNF_KEY_IsKeyUp failed, ret = 0x%x\n", s32Ret);
            goto ERR1;
        }

        g_bTaskQuit = MT_FALSE;
        /* create a thread for receive */
        s32Ret = pthread_create(&g_keyTaskid, NULL, (MT_VOID * (*)(MT_VOID *))MT_Receive, NULL);
        if (0 != s32Ret)
        {
            SAMPLE_KADC_ERR_PRINT("pthread_create failed, ret = 0x%x\n", s32Ret);
            goto ERR1;
        }
    }

    (MT_VOID)MT_KadcCmdTask();

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

    g_bTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_keyTaskid, 0);

ERR1:
    (MT_VOID)MT_UNF_KEYLED_DeInit();
ERR0:
#ifndef MT_SAMPLE_APP
    (MT_VOID)mt_sys_deinit();
#endif

    return s32Ret;
}



