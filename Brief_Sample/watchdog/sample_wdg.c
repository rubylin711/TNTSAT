#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mt_unf_wdg.h"
#include <pthread.h>
#include "mt_adp_mpi.h"


#ifdef MT_SAMPLE_WDG_DEBUG

#define MT_WDG_PRINT   printf
#else

#define MT_WDG_PRINT

#endif

#define SAMPLE_WDG_FUNCTION_ENTER()     MT_WDG_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_WDG_FUNCTION_EXIT()      MT_WDG_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_WDG_FATAL_PRINT(fmt...)          MT_WDG_PRINT(" [FATAL] " fmt)
#define SAMPLE_WDG_ERR_PRINT(fmt...)            MT_WDG_PRINT(" [ERROR] " fmt)
#define SAMPLE_WDG_WARN_PRINT(fmt...)           MT_WDG_PRINT(" [WARN] "  fmt)
#define SAMPLE_WDG_INFO_PRINT(fmt...)           MT_WDG_PRINT(" [INFO] "  fmt)
#define SAMPLE_WDG_DBG_PRINT(fmt...)            MT_WDG_PRINT(" [DEBUG] " fmt)

#define SAMPLE_WDG_PRINT   printf

#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2

#define WDG_NO  0

typedef struct
{
    pthread_t  wdg;
} MT_WDG_RUN_INFO;

static MT_BOOL g_bTaskQuit = MT_TRUE;
static MT_BOOL g_bStopThread = MT_TRUE;


static MT_WDG_RUN_INFO    g_stWdgRunInfo;

#ifdef MT_SAMPLE_APP
MT_S32 MT_WdgMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

static mt_s32 *MT_WdgFeedTask(void *param)
{
    mt_s32 s32Ret = 0;

    while(g_bStopThread == MT_FALSE)
    {
        /* Clear WDG during timeout, can not reset system */
        s32Ret = mt_unf_wdg_clear(WDG_NO);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_WDG_ERR_PRINT("mt_unf_wdg_clear err! s32Ret = 0x%x\n",s32Ret);
        }

        SAMPLE_WDG_INFO_PRINT("Clear wdg Success\n");
        sleep(1);
    }
    return 0;
}
static MT_VOID MT_WdgPrintMenu(MT_VOID)
{

    SAMPLE_WDG_PRINT("     s : stop to feed dog \n");
#ifdef MT_SAMPLE_APP
    SAMPLE_WDG_PRINT("     b : background run \n");
#endif
    SAMPLE_WDG_PRINT("     h : help \n");
    SAMPLE_WDG_PRINT("     q : quit \n");
    SAMPLE_WDG_PRINT("WDG>> ");

}

static void MT_WdgCmdTask(void)
{
    char *fgetret = NULL;
    MT_CHAR inputCmd[32] = { 0 };


    while (1)
    {
        (void)MT_WdgPrintMenu();

        fgetret = fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        fgetret = fgetret;


        if('q' == inputCmd[0])
        {
            SAMPLE_WDG_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_WDG_INFO_PRINT("wdg in back!\n");
            break;
        }
#endif
        else if('s' == inputCmd[0])
        {
            SAMPLE_WDG_INFO_PRINT("stop to feed,The system is about to restart.\n");
            sleep(1);
            g_bStopThread = MT_TRUE;
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_WDG_INFO_PRINT("Print help info \n");
            continue;
        }

    }
}

static MT_VOID MT_WdgExit(void)
{
    g_bTaskQuit = MT_TRUE;
    if(g_bStopThread == MT_FALSE)
    {
        g_bStopThread = MT_TRUE;
        (MT_VOID)pthread_join(g_stWdgRunInfo.wdg, NULL);
    }
    (MT_VOID)mt_unf_wdg_disable(WDG_NO);
    (MT_VOID)mt_unf_wdg_deinit();
    memset(&g_stWdgRunInfo, 0xff, sizeof(g_stWdgRunInfo));
}

/*!
@brief gets the external input parameters
@param[in]  argc            The number of external input parameters
@param[in]  argv            External input parameter values
@return::MT_VOID
@*/
static MT_S32 MT_WdgParase_args(MT_S32 argc, MT_CHAR *argv[])
{
    int opt = 0;

    SAMPLE_WDG_FUNCTION_ENTER();

    while((opt = MTADP_Getopt(argc, argv, ":?hH:q")) != -1)
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
                    (MT_VOID)MT_WdgExit();
                }
                return MT_TASK_EXIT;


            default:
                return MT_FAILURE;
            break;
        }
    }

    SAMPLE_WDG_FUNCTION_EXIT();

    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_WdgMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    mt_s32 s32Ret = 0;


    if(argc != 1 && g_bTaskQuit == MT_TRUE)
    {
        return MT_SUCCESS;
    }

    /** Get the parameters */
    s32Ret = MT_WdgParase_args(argc, argv);
    if (MT_FAILURE == s32Ret)
    {
        SAMPLE_WDG_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == s32Ret)
    {
        SAMPLE_WDG_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }
    if(g_bTaskQuit == MT_TRUE)
    {
        /* Open WDG*/
        s32Ret = mt_unf_wdg_init();
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_WDG_ERR_PRINT("mt_unf_wdg_init err! s32Ret = 0x%x\n",s32Ret);
            return s32Ret;
        }

        /* Set WDG TimeOut */
        s32Ret = mt_unf_wdg_set_timeout(WDG_NO, 2000);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_WDG_ERR_PRINT("mt_unf_wdg_set_timeout err! s32Ret = 0x%x\n",s32Ret);
            (MT_VOID)mt_unf_wdg_deinit();
        }

        /* Enable WDG */
        s32Ret = mt_unf_wdg_enable(WDG_NO);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_WDG_ERR_PRINT("mt_unf_wdg_enable err! s32Ret = 0x%x\n",s32Ret);
            (MT_VOID)mt_unf_wdg_deinit();
        }
        g_bTaskQuit = MT_FALSE;
        s32Ret = pthread_create(&g_stWdgRunInfo.wdg, NULL, (void * (*)(void *))MT_WdgFeedTask, NULL);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_WDG_ERR_PRINT("pthread_create ret = %d\n", s32Ret);
            goto ERR1;
        }
        g_bStopThread = MT_FALSE;
    }

    (void)MT_WdgCmdTask();
    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }
ERR1:
    (MT_VOID)MT_WdgExit();

    return s32Ret;
}

