/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <mt_unf_keyled.h>
#include "mt_unf_demux.h"
#include "mt_adp_mpi.h"
#include "mt_adp_frontend.h"
#include "mt_adp_hdmi.h"
#include "pthread.h"
#include "mt_unf_misc.h"
#include <sys/ioctl.h>
#include "mt_unf_pm.h"
#include "mt_unf_ir.h"
/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_TEMP_DEBUG
#define MT_TEMP_PRINT   printf
#else
#define MT_TEMP_PRINT
#endif

#define SAMPLE_TEMP_FUNCTION_ENTER()            MT_TEMP_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_TEMP_FUNCTION_EXIT()             MT_TEMP_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_TEMP_FATAL_PRINT(fmt...)         MT_TEMP_PRINT(" [FATAL] " fmt)
#define SAMPLE_TEMP_ERR_PRINT(fmt...)           MT_TEMP_PRINT(" [ERROR] " fmt)
#define SAMPLE_TEMP_WARN_PRINT(fmt...)          MT_TEMP_PRINT(" [WARN] "  fmt)
#define SAMPLE_TEMP_INFO_PRINT(fmt...)          MT_TEMP_PRINT(" [INFO] "  fmt)
#define SAMPLE_TEMP_DBG_PRINT(fmt...)           MT_TEMP_PRINT(" [DEBUG] " fmt)

#define SAMPLE_TEMP_PRINT   printf

#define MT_TASK_RUN         1
#define MT_TASK_EXIT        2
/*************************** Structure Definition ****************************/
typedef struct
{
    pthread_t          stInjectTSThread;
} MT_TEMP_RUN_INFO;
/********************** Global Variable declaration **************************/
static MT_BOOL   g_bTaskQuit = MT_TRUE;
static MT_BOOL   g_printQuit = MT_TRUE;
static MT_TEMP_RUN_INFO temp_run_info;
static pthread_t g_printThread;

/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_TemperatureMain(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

/*****************************************************************************
*brief Enter Real  standby
*param  void
*return::MT_SUCCESS             Success.
*return::MT_FAILURE             Fail.
*****************************************************************************/
static MT_VOID MT_TempPowerIrStandby(MT_VOID)
{
    MT_S32 s32Ret = 0;
    standby_time_t gtime = { 0 };
    ir_wavefilter_config_s wavefiler = { 0 };
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    MT_U8 protocol = IRDA_NEC;
#elif defined CONFIG_MT_CHIP_SYMPHONY6
    MT_U8 protocol = RC_PROTO_NEC_;
#endif

    wavefiler.irda_protocol = protocol;
    wavefiler.irda_wfilt_channel = 4;


#ifdef CONFIG_MT_CHIP_SYMPHONY4
    wavefiler.irda_wfilt_channel_cfg[0].addr_len = 32;
    wavefiler.irda_wfilt_channel_cfg[0].wfilt_code = 0x7F800AF5;
    wavefiler.irda_wfilt_channel_cfg[0].protocol = IRDA_NEC;
    wavefiler.irda_wfilt_channel_cfg[1].addr_len = 32;
    wavefiler.irda_wfilt_channel_cfg[1].wfilt_code = 0xfd0148b7;
    wavefiler.irda_wfilt_channel_cfg[1].protocol = IRDA_NEC;
    (MT_VOID)MT_UNF_IR_SetWaveFilter(wavefiler);
    (MT_VOID)MT_UNF_IR_SetKeycode(0);
    (MT_VOID)MT_UNF_IR_SetUsercode(0);

#elif defined CONFIG_MT_CHIP_SYMPHONY6
    wavefiler.irda_wfilt_channel_cfg[1].protocol = RC_PROTO_NEC_;
    wavefiler.irda_wfilt_channel_cfg[1].addr_len = 32;
    wavefiler.irda_wfilt_channel_cfg[1].wfilt_code = 0x800a;

    wavefiler.irda_wfilt_channel_cfg[0].protocol = RC_PROTO_NECX_;
    wavefiler.irda_wfilt_channel_cfg[0].addr_len = 32;
    wavefiler.irda_wfilt_channel_cfg[0].wfilt_code = 0x1fd48;

    (MT_VOID)MT_UNF_IR_SetWaveFilter(&wavefiler);
#endif

    s32Ret = MT_UNF_PMOC_Init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_TEMP_ERR_PRINT("MT_UNF_PMOC_Init failed\n");
    }

    SAMPLE_TEMP_INFO_PRINT("gtime[0x%x][0x%x][0x%x] day[0x%x] \n", gtime.cur_hour, gtime.cur_min, gtime.cur_sec, gtime.pass_day);

    s32Ret = MT_UNF_PMOC_SetDevType(3);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_TEMP_ERR_PRINT("MT_UNF_PMOC_SetDevType failed\n");
    }

	SAMPLE_TEMP_INFO_PRINT("MT_UNF_PMOC_SetGpenPin 0..\n");
	s32Ret = MT_UNF_PMOC_SetGpenPin(0);	
	if(MT_SUCCESS != s32Ret)
	{
		SAMPLE_TEMP_ERR_PRINT("MT_UNF_PMOC_SetGpenPin  failed\n");
	}

	s32Ret = MTADP_Switch_Standby_Mode();
	if(MT_SUCCESS != s32Ret)
	{
		SAMPLE_TEMP_ERR_PRINT("MTADP_Switch_Standby_Mode  failed\n");
	}

    SAMPLE_TEMP_INFO_PRINT(" start low power......\n");
    s32Ret = MT_UNF_PMOC_SwitchSystemMode();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_TEMP_ERR_PRINT("MT_UNF_PMOC_SwitchSystemMode  failed\n");
    }
}

#ifdef CONFIG_MT_CHIP_SYMPHONY4
/*!
@brief The thread that receives the file stream data
@param[in] args                 TS file name
@return::MT_SUCCESS             Success.
@return::s32Ret                 The return value of the error.
@*/
static MT_S32 InjectTsTask(MT_VOID *args)
{
    MT_S32  s32Ret = MT_SUCCESS;
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    mt_s32  temperature = 0;
#elif defined CONFIG_MT_CHIP_SYMPHONY6
    long  temperature = 0;
#endif

    /* loop in inject data */
    while(g_bTaskQuit == MT_FALSE)
    {
        s32Ret = mt_unf_misc_temperature_get(&temperature);
        if(MT_SUCCESS == s32Ret)
        {
            temperature = temperature / 1000;
            SAMPLE_TEMP_INFO_PRINT("Current temperature: %ld\n", temperature);
        }
        else
        {
            SAMPLE_TEMP_ERR_PRINT("Failed to get temperature!\n");
        }

        if(temperature > 120)
        {
            usleep(200);
            SAMPLE_TEMP_WARN_PRINT("The temperature is too high and it is about to enter standby\n");
            sleep(5);
            (MT_VOID)MT_TempPowerIrStandby();
        }

        sleep(2);
    }

    return MT_SUCCESS;
}
#endif
static MT_VOID MT_TempExit(MT_VOID)
{
    g_bTaskQuit = MT_TRUE;

#ifdef CONFIG_MT_CHIP_SYMPHONY4
    (MT_VOID)pthread_join(temp_run_info.stInjectTSThread, NULL);
#endif
    (MT_VOID)MT_UNF_IR_DeInit();

    memset(&temp_run_info, 0xff, sizeof(temp_run_info));

    (MT_VOID)mt_unf_misc_release();
}

static MT_VOID MT_TempPrintMenu(MT_VOID)
{
    SAMPLE_TEMP_PRINT("     o : print current temperature \n");
#ifdef MT_SAMPLE_APP
    SAMPLE_TEMP_PRINT("     b : background run \n");
#endif
    SAMPLE_TEMP_PRINT("     h : help \n");
    SAMPLE_TEMP_PRINT("     q : quit \n");
    SAMPLE_TEMP_PRINT("TEMP>> ");
}

#ifdef CONFIG_MT_CHIP_SYMPHONY6
static int temp_sys_monitor_callback(MT_SYS_EVENT_T event, void *data)
{
    static int conunt = 0;

    switch (event)
    {
        case MT_SYS_EVENT_TEMP:
            //printf("%s: event %d, temp %d\r\n", __FUNCTION__, event, temp);
            break;

        case MT_SYS_EVENT_TEMP_HIGH_YELLOW:
            printf("TEMPERATURE_HIGH_YELLOW event!\n");
            break;

        case MT_SYS_EVENT_TEMP_HIGH_RED:
            printf("TEMPERATURE_HIGH_RED event!\n");
            conunt++;
            if(3 == conunt)
            {
                SAMPLE_TEMP_WARN_PRINT("Continuous high temperature, enter standby mode!\n");
                (MT_VOID)MT_TempPowerIrStandby();
            }
            break;

        default:
            break;
    }

    return 0;
}
#endif

static void task_print_temperature(void *args)
    {
        MT_S32  s32Ret = MT_SUCCESS;
#ifdef CONFIG_MT_CHIP_SYMPHONY4
        mt_s32  temperature = 0;
#elif defined CONFIG_MT_CHIP_SYMPHONY6
        long  temperature = 0;
#endif

        /* loop in inject data */
        while(g_printQuit != MT_TRUE)
        {
            s32Ret = mt_unf_misc_temperature_get(&temperature);
            if(MT_SUCCESS == s32Ret)
            {
                temperature = temperature / 1000;
                SAMPLE_TEMP_INFO_PRINT("Current temperature: %ld\n", temperature);
            }
            else
            {
                SAMPLE_TEMP_ERR_PRINT("Failed to get temperature!\n");
            }

            sleep(2);
        }

        return;
    }


/*!
@brief The thread on which the command was entered
@param[in]  hAvplay             Handle to AV player
@param[in]  hWin                The input window handler
@param[in]  pProgTbl            The data structure of the PMT
@return::MT_VOID
@*/
static MT_VOID MT_TempCmdTask(MT_VOID)
{
    MT_S32                 s32Ret = MT_SUCCESS;
    MT_CHAR inputCmd[32] = { 0 };

    while(1)
    {
        (MT_VOID)MT_TempPrintMenu();

        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        if('q' == inputCmd[0])
        {
            SAMPLE_TEMP_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
        else if('o' == inputCmd[0])
        {
            if (g_printQuit != MT_FALSE) {
                SAMPLE_TEMP_INFO_PRINT("Turn on temperature printing !!! \n");
                g_printQuit = MT_FALSE;
                s32Ret = pthread_create(&g_printThread, NULL, (MT_VOID * (*)(MT_VOID *))task_print_temperature, NULL);
                if (MT_SUCCESS != s32Ret) {
                    SAMPLE_TEMP_ERR_PRINT("failed to pthread_create\n");
                }
            } else {
                SAMPLE_TEMP_INFO_PRINT("Turn off playback duration printing !!! \n");
                g_printQuit = MT_TRUE;
                pthread_join(g_printThread, NULL);
            }
            continue;
        }
    #ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_TEMP_INFO_PRINT("temp play in back!\n");
            break;
        }
    #endif
        else if('h' == inputCmd[0])
        {
            SAMPLE_TEMP_INFO_PRINT("Print help info \n");
            continue;
        }
    }
}

/*!
@brief Help information
@param[in]  name     The executable name
@return::MT_VOID
@*/
static MT_VOID MT_TempPrint_Help(MT_CHAR *name)
{
    SAMPLE_TEMP_PRINT("Lack of parameters\n");
    SAMPLE_TEMP_PRINT("\nUsage:\n");
    SAMPLE_TEMP_PRINT("%s\n", name);
#ifdef MT_SAMPLE_APP
    SAMPLE_TEMP_PRINT("    -q: Exit the background\n");
#endif
    SAMPLE_TEMP_PRINT("example:\n");
    SAMPLE_TEMP_PRINT("    %s -q\n", name);
}


/*!
@brief gets the external input parameters
@param[in]  argc            The number of external input parameters
@param[in]  argv            External input parameter values
@param[out] pstCmd          Gets the address of the file path
@return::MT_VOID
@*/
static MT_S32 MT_TempParase_args(MT_S32 argc, MT_CHAR *argv[])
{
    MT_S32 opt = 0;

    while((opt = MTADP_Getopt(argc, argv, ":?hH:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_TempPrint_Help(argv[0]);
                return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_TempExit();
                }
                return MT_TASK_EXIT;
            default:
                (MT_VOID)MT_TempPrint_Help(argv[0]);
                return MT_FAILURE;
        }
    }

    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_TemperatureMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    MT_S32                 s32Ret = MT_SUCCESS;
    long temperature;

    s32Ret = MT_TempParase_args(argc, argv);
    if (MT_FAILURE == s32Ret)
    {
        SAMPLE_TEMP_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == s32Ret)
    {
        SAMPLE_TEMP_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
    #ifndef MT_SAMPLE_APP
        /** System initialization */
        s32Ret = mt_sys_init();
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_TEMP_ERR_PRINT("failed to mt_sys_init\n");
            return s32Ret;
        }

        /** HDMI initialization */
        s32Ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_TEMP_ERR_PRINT("failed to MTADP_HDMI_Init\n");
            goto ERR0;
        }

        sleep(1);

        /** Display initialization */
        s32Ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_TEMP_ERR_PRINT("failed to MTADP_Disp_Init\n");
            goto ERR1;
        }
    #endif

        g_bTaskQuit = MT_FALSE;

        s32Ret = mt_unf_misc_init();
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_TEMP_ERR_PRINT("failed to mt_unf_misc_init\n");
            goto ERR2;
        }

        s32Ret = mt_unf_misc_temperature_get(&temperature);
        if(MT_SUCCESS == s32Ret)
        {
            temperature = temperature / 1000;
            SAMPLE_TEMP_INFO_PRINT("Current temperature: %ld\n", temperature);
        }

        s32Ret = MT_UNF_IR_Init();
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_TEMP_ERR_PRINT("MT_UNF_IR_Init ret = %d\n", s32Ret);
            goto ERR3;
        }
#ifdef CONFIG_MT_CHIP_SYMPHONY4
        s32Ret = MT_UNF_IR_Enable(MT_TRUE, IRDA_NEC);
#elif defined CONFIG_MT_CHIP_SYMPHONY6
        s32Ret = MT_UNF_IR_Enable(MT_TRUE, RC_PROTO_NEC_);
#endif
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_TEMP_ERR_PRINT("MT_UNF_IR_SetFetchMode ret = %d\n", s32Ret);
            goto ERR4;
        }
#ifdef CONFIG_MT_CHIP_SYMPHONY4
        /** Create a thread to read the TS stream file */
        s32Ret = pthread_create(&temp_run_info.stInjectTSThread, MT_NULL, (MT_VOID * (*)(MT_VOID *))InjectTsTask, MT_NULL);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_TEMP_ERR_PRINT("failed to pthread_create\n");
            goto ERR4;
        }
#elif defined CONFIG_MT_CHIP_SYMPHONY6
        mt_unf_sys_monitor_register(temp_sys_monitor_callback);
#endif
    }

    (MT_VOID)MT_TempCmdTask();

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

    if (g_printQuit != MT_TRUE)
    {
        g_printQuit = MT_TRUE;
        pthread_join(g_printThread, NULL);
        g_printThread = -1;
    }

#ifdef CONFIG_MT_CHIP_SYMPHONY4
    (MT_VOID)pthread_join(temp_run_info.stInjectTSThread, NULL);
#endif
ERR4:
    (MT_VOID)MT_UNF_IR_DeInit();
    memset(&temp_run_info, 0xff, sizeof(temp_run_info));
ERR3:
    (MT_VOID)mt_unf_misc_release();
ERR2:
#ifndef MT_SAMPLE_APP
    /** Display deinitialization */
    (MT_VOID)MTADP_Disp_DeInit();
ERR1:
    /** HDMI deinitialization */
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
ERR0:
    /** system deinitialized */
    (MT_VOID)mt_sys_deinit();
#endif
    g_bTaskQuit = MT_TRUE;

    return s32Ret;
}
