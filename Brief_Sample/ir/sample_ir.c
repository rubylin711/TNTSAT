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
#include "mt_type.h"
#include <pthread.h>
#include "mt_adp_hdmi.h"
#include "mt_adp_mpi.h"





/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_IR_DEBUG

#define MT_IR_PRINT   printf
#else

#define MT_IR_PRINT

#endif

#define SAMPLE_IR_FUNCTION_ENTER()  MT_IR_PRINT("[IR][%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_IR_FUNCTION_EXIT()       MT_IR_PRINT("[IR][%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_IR_FATAL_PRINT(fmt...)       MT_IR_PRINT(" [IR][FATAL] " fmt)
#define SAMPLE_IR_ERR_PRINT(fmt...)         MT_IR_PRINT(" [IR][ERROR] " fmt)
#define SAMPLE_IR_WARN_PRINT(fmt...)        MT_IR_PRINT(" [IR][WARN] "  fmt)
#define SAMPLE_IR_INFO_PRINT(fmt...)        MT_IR_PRINT(" [IR][INFO] "  fmt)
#define SAMPLE_IR_DBG_PRINT(fmt...)         MT_IR_PRINT(" [IR][DEBUG] "

#define SAMPLE_IR_PRINT printf

#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2


typedef struct
{
    pthread_t  key;
} MT_IR_RUN_INFO;

static MT_BOOL g_bTaskQuit = MT_TRUE;

static MT_IR_RUN_INFO    g_stIrRunInfo;


#ifdef MT_SAMPLE_APP
MT_S32 MT_IrMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

/*
@brief Obtain the key value of the remote control and configure the infrared protocol of the remote control
@@return MT_SUCCESS
@return MT_FAILURE
*/
static mt_s32 MT_Ir_key(void)
{
    mt_s32 Ret =  0;
    MT_UNF_KEY_STATUS_E press_status = { 0 };
    MT_U64 u64KeyId = 0;
    char name[64] = { 0 };
    ir_wavefilter_config_s wavefiler = { 0 };
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    MT_U8 protocol = IRDA_SW_MAX;
#elif defined CONFIG_MT_CHIP_SYMPHONY6
    MT_U8 protocol = RC_PROTO_MAX_;
#endif


    SAMPLE_IR_INFO_PRINT("Use the remote control to align the board to get the key value\n");
    while(g_bTaskQuit != MT_TRUE)
    {
        Ret = MT_UNF_IR_GetValueWithProtocol(&press_status, &u64KeyId, name, sizeof(name),3000);
        if(MT_SUCCESS == Ret)
        {
            SAMPLE_IR_INFO_PRINT("press_status = %d, u64KeyId = 0x%llx, name = %s\n", press_status, u64KeyId, name);
#ifdef CONFIG_MT_CHIP_SYMPHONY4
            if(0 == strcmp(name, "IRDA_NEC"))
            {
                protocol = IRDA_NEC;
            }
#elif defined CONFIG_MT_CHIP_SYMPHONY6
            if(0 == strcmp(name, "nec"))
            {
                protocol = RC_PROTO_NEC_;
            }
#endif
            else
            {
                continue;
            }
            SAMPLE_IR_INFO_PRINT("protocol = %d\n", protocol);

            wavefiler.irda_protocol = protocol;
            wavefiler.irda_wfilt_channel = 4;

#ifdef CONFIG_MT_CHIP_SYMPHONY4
            Ret = MT_UNF_IR_SetWaveFilter(wavefiler);
            if (MT_SUCCESS != Ret)
            {
                SAMPLE_IR_ERR_PRINT("MT_UNF_IR_SetWaveFilter Ret = %d\n", Ret);
                return Ret;
            }
            Ret = MT_UNF_IR_SetKeycode(0);
            if (MT_SUCCESS != Ret)
            {
                SAMPLE_IR_ERR_PRINT("MT_UNF_IR_SetKeycode Ret = %d\n", Ret);
                return Ret;
            }
            Ret = MT_UNF_IR_SetUsercode(0);
            if (MT_SUCCESS != Ret)
            {
                SAMPLE_IR_ERR_PRINT("MT_UNF_IR_SetUsercode Ret = %d\n", Ret);
                return Ret;
            }
#elif defined CONFIG_MT_CHIP_SYMPHONY6
            Ret = MT_UNF_IR_SetWaveFilter(&wavefiler);
            if (MT_SUCCESS != Ret)
            {
                SAMPLE_IR_ERR_PRINT("MT_UNF_IR_SetWaveFilter Ret = %d\n", Ret);
                return Ret;
            }
#endif
        }
        usleep(5000);
    }
    return 0;
}

static MT_VOID MT_IrPrintMenu(void)
{

#ifdef MT_SAMPLE_APP
    SAMPLE_IR_PRINT("     b : background run \n");
#endif
    SAMPLE_IR_PRINT("     h : help \n");
    SAMPLE_IR_PRINT("     q : quit \n");
    SAMPLE_IR_PRINT("IR>> ");

}

/*
@brief quit
@return void
*/
static void MT_IrCmdTask(void)
{
    char *fgetret = NULL;
    MT_CHAR inputCmd[32] = { 0 };

    while (1)
    {
        MT_IrPrintMenu();
        fgetret = fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        fgetret = fgetret;


        if ('q' == inputCmd[0])
        {
            SAMPLE_IR_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
        #ifdef MT_SAMPLE_APP
        else if('b' == inputCmd[0])
        {
            SAMPLE_IR_INFO_PRINT("ir in back!\n");
            break;
        }
        #endif
        else if('h' == inputCmd[0])
        {
            SAMPLE_IR_INFO_PRINT("help info\n");
        }
    }
}

static MT_VOID MT_IrExit(MT_VOID)
{

    g_bTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_stIrRunInfo.key, NULL);

    (MT_VOID)MT_UNF_IR_DeInit();
    memset(&g_stIrRunInfo, 0xff, sizeof(g_stIrRunInfo));
}

static mt_s32 MT_IrParase_args(int argc, char *argv[])
{
    int opt = 0;


    SAMPLE_IR_FUNCTION_ENTER();

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
                   (MT_VOID)MT_IrExit();
                }
                return MT_TASK_EXIT;


            default:
                return MT_FAILURE;
            break;
        }
    }
    SAMPLE_IR_FUNCTION_EXIT();
    return MT_SUCCESS;
}

#ifdef MT_SAMPLE_APP
MT_S32 MT_IrMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    MT_S32 ret = MT_FAILURE;
    MT_UNF_KEY_STATUS_E press_status = MT_UNF_KEY_STATUS_BUTT;
    MT_U64 u64KeyId = 0;
    char name[64] = { 0 };

    if(argc != 1 && g_bTaskQuit == MT_TRUE)
    {
        return MT_SUCCESS;
    }
    ret = MT_IrParase_args(argc, argv);
    if (MT_FAILURE == ret)
    {
        SAMPLE_IR_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_IR_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if (MT_SUCCESS != ret)
        {
            SAMPLE_IR_ERR_PRINT("mt_sys_init error. ret=0x%x \n", ret);
            return ret;
        }

#endif
        ret = MT_UNF_IR_Init();
        if (MT_SUCCESS != ret)
        {
            SAMPLE_IR_ERR_PRINT("MT_UNF_IR_Init ret = %d\n", ret);
            goto ERR1;
        }

        ret = MT_UNF_IR_SetRepKeyTimeoutAttr(500);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_IR_ERR_PRINT("MT_UNF_IR_SetRepKeyTimeoutAttr ret = %d\n", ret);
            goto ERR2;
        }

        ret = MT_UNF_IR_EnableKeyUp(MT_FALSE);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_IR_ERR_PRINT("MT_UNF_IR_EnableKeyUp ret = %d\n", ret);
            goto ERR2;
        }

        ret = MT_UNF_IR_EnableRepKey(MT_FALSE);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_IR_ERR_PRINT("MT_UNF_IR_EnableRepKey ret = %d\n", ret);
            goto ERR2;
        }

        ret = MT_UNF_IR_SetFetchMode(0);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_IR_ERR_PRINT("MT_UNF_IR_SetFetchMode ret = %d\n", ret);
            goto ERR2;
        }
#ifdef CONFIG_MT_CHIP_SYMPHONY4
        ret = MT_UNF_IR_Enable(MT_TRUE, IRDA_NEC);
#elif defined CONFIG_MT_CHIP_SYMPHONY6
        ret = MT_UNF_IR_Enable(MT_TRUE, RC_PROTO_NEC_);
#endif
        if (MT_SUCCESS != ret)
        {
            SAMPLE_IR_ERR_PRINT("MT_UNF_IR_Enable ret = %d\n", ret);
            goto ERR2;
        }
        while(MT_SUCCESS == MT_UNF_IR_GetValueWithProtocol(&press_status, &u64KeyId,name, sizeof(name),1000)) //Clear IR DataCache
        {
            MT_USLEEP(5000);
        }
        g_bTaskQuit = MT_FALSE;
        ret = pthread_create(&g_stIrRunInfo.key, NULL, (void * (*)(void *))MT_Ir_key, NULL);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_IR_ERR_PRINT("pthread_create ret = %d\n", ret);
            goto ERR2;
        }
    }

    (void)MT_IrCmdTask();
    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

    g_bTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_stIrRunInfo.key, NULL);

ERR2:
    (MT_VOID)MT_UNF_IR_DeInit();
ERR1:
#ifndef MT_SAMPLE_APP
    (MT_VOID)mt_sys_deinit();

#endif
    memset(&g_stIrRunInfo, 0xff, sizeof(g_stIrRunInfo));
    return 0;
}

