/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/*********************************add include here******************************/

#include <fcntl.h>
#include <unistd.h>

#include "mt_unf_common.h"
#include "mt_type.h"
#include "mt_debug.h"

#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <mt_adp_mpi.h>






/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_SYSTEM_DEBUG

#define MT_SYSTEM_PRINT   printf
#else

#define MT_SYSTEM_PRINT

#endif

#define SAMPLE_SYSTEM_FUNCTION_ENTER()      MT_SYSTEM_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_SYSTEM_FUNCTION_EXIT()       MT_SYSTEM_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_SYSTEM_FATAL_PRINT(fmt...)      MT_SYSTEM_PRINT(" [FATAL] " fmt)
#define SAMPLE_SYSTEM_ERR_PRINT(fmt...)        MT_SYSTEM_PRINT(" [ERROR] " fmt)
#define SAMPLE_SYSTEM_WARN_PRINT(fmt...)       MT_SYSTEM_PRINT(" [WARN] "  fmt)
#define SAMPLE_SYSTEM_INFO_PRINT(fmt...)       MT_SYSTEM_PRINT(" [INFO] "  fmt)
#define SAMPLE_SYSTEM_DBG_PRINT(fmt...)        MT_SYSTEM_PRINT(" [DEBUG] " fmt)

#define SAMPLE_SYSTEM_PRINT   printf


#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2




typedef struct
{
    pthread_t   hinfoThd;
    MT_BOOL thdRun;

} MT_System_RUN_INFO;





/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;

static MT_System_RUN_INFO    g_stSystemRunInfo;





/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_System_InfoMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif




static void MT_System_Info_Print(mt_void *args)
{
    mt_s32 time = 0;


    time =  *((int *)args);
    while(g_bTaskQuit == MT_FALSE)
    {
        system("top -b -n 1");
        system("cat /proc/meminfo");
        sleep(time);

    }
    SAMPLE_SYSTEM_INFO_PRINT("get info over !! \n");
}


static MT_VOID MT_System_InfoPrintMenu(MT_VOID)
{
    SAMPLE_SYSTEM_PRINT("      g : Get system info \n");
    SAMPLE_SYSTEM_PRINT("      s : Set wait time \n");
#ifdef MT_SAMPLE_APP
    SAMPLE_SYSTEM_PRINT("      b : background run \n");
#endif
    SAMPLE_SYSTEM_PRINT("      h : Help information \n");
    SAMPLE_SYSTEM_PRINT("      q : exit  \n");
    SAMPLE_SYSTEM_PRINT("System_info>> ");
}

static mt_s32 MT_System_InfoCmdTask(MT_VOID)
{
    mt_s32     ret = MT_SUCCESS;
    MT_CHAR    inputCmd[32] = { 0 };
    mt_s32     wait_time = 10;


    while (1)
    {
        (void)MT_System_InfoPrintMenu();

        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        /* quit*/
        if('q' == inputCmd[0])
        {
            SAMPLE_SYSTEM_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_SYSTEM_INFO_PRINT("system info play in back!\n");
            break;
        }
#endif
        else if('s' == inputCmd[0])
        {
            SAMPLE_SYSTEM_INFO_PRINT("please input wait time \n");
            scanf("%d", &wait_time);
            SAMPLE_SYSTEM_INFO_PRINT("wait_time = %d \n", wait_time);
        }


        else if('g' == inputCmd[0])
        {
            ret = pthread_create(&g_stSystemRunInfo.hinfoThd, NULL, (void * (*)(void *))MT_System_Info_Print, (void *)&wait_time);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_SYSTEM_ERR_PRINT("failed to pthread_create\n");
            }
            g_stSystemRunInfo.thdRun = MT_TRUE;
        }

        else if('h' == inputCmd[0])
        {
            SAMPLE_SYSTEM_INFO_PRINT("Print help info \n");
            continue;
        }

    }
    return MT_SUCCESS;
}
static void MT_System_InfoExit(MT_VOID)
{
    g_bTaskQuit = MT_TRUE;
    if(g_stSystemRunInfo.thdRun == MT_TRUE)
    {
        (void)pthread_join(g_stSystemRunInfo.hinfoThd, NULL);
        g_stSystemRunInfo.thdRun = MT_FALSE;
    }

    memset(&g_stSystemRunInfo, 0, sizeof(g_stSystemRunInfo));
}



/*
 @brief Get input parameters according to the conditions
 @param[in] argc  The number of parameters entered
 @param[in] argv  Input parameter
 @return ::MT_SUCCESS
*/
static MT_S32 MT_System_InfoParase_args(int argc, char *argv[])
{
    int opt = 0;


    while((opt = MTADP_Getopt(argc, argv, "h?H:q")) != -1)
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
                    (MT_VOID)MT_System_InfoExit();
                }
                return MT_TASK_EXIT;

            default:
                return MT_FAILURE;
        }
    }


    return MT_SUCCESS;

}


#ifdef MT_SAMPLE_APP
MT_S32 MT_System_InfoMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    mt_s32     ret = MT_SUCCESS;



#ifndef MT_SAMPLE_APP
    ret = mt_sys_init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SYSTEM_ERR_PRINT("mt_sys_init err! \n");
        return MT_FAILURE;
    }
#endif

    ret = MT_System_InfoParase_args(argc, argv);
    if(MT_FAILURE == ret)
    {
        SAMPLE_SYSTEM_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_SYSTEM_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    g_bTaskQuit = MT_FALSE;



    (MT_VOID)MT_System_InfoCmdTask();

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

    if(g_stSystemRunInfo.thdRun == MT_TRUE)
    {
        (MT_VOID)pthread_join(g_stSystemRunInfo.hinfoThd, NULL);
        g_stSystemRunInfo.thdRun = MT_FALSE;
    }


#ifndef MT_SAMPLE_APP
    (MT_VOID)mt_sys_deinit();
#endif
    g_bTaskQuit = MT_TRUE;

    memset(&g_stSystemRunInfo, 0, sizeof(g_stSystemRunInfo));
    return MT_SUCCESS;
}
