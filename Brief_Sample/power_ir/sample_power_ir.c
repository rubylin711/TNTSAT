/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mt_unf_ir.h>
#include <mt_unf_keyled.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include "mt_unf_pm.h"
#include "mt_unf_ir.h"
#include "mt_adp_mpi.h"
#include "mt_adp_frontend.h"

/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_POWER_IR_DEBUG

#define MT_POWER_IR_PRINT   printf
#else

#define MT_POWER_IR_PRINT

#endif

#define SAMPLE_POWER_IR_FUNCTION_ENTER()           MT_POWER_IR_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_POWER_IR_FUNCTION_EXIT()            MT_POWER_IR_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_POWER_IR_FATAL_PRINT(fmt...)         MT_POWER_IR_PRINT(" [FATAL] " fmt)
#define SAMPLE_POWER_IR_ERR_PRINT(fmt...)           MT_POWER_IR_PRINT(" [ERROR] " fmt)
#define SAMPLE_POWER_IR_WARN_PRINT(fmt...)          MT_POWER_IR_PRINT(" [WARN] "  fmt)
#define SAMPLE_POWER_IR_INFO_PRINT(fmt...)          MT_POWER_IR_PRINT(" [INFO] "  fmt)
#define SAMPLE_POWER_IR_DBG_PRINT(fmt...)           MT_POWER_IR_PRINT(" [DEBUG] " fmt)

#define SAMPLE_POWER_IR_PRINT printf

#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2

/*************************** Structure Definition ****************************/


typedef struct
{
    pthread_t  key;
} MT_Power_IR_RUN_INFO;



/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;
static MT_Power_IR_RUN_INFO    g_stPower_irRunInfo;


/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_Power_IrMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

static void MT_Power_Frontend_enter_standby_sym6(void)
{
    mt_u32 tuner_id = 0;
    mt_unf_fe_attr_t fe_attr;
    mt_u8 cs8800_standby_enable_flag = 0;/*0:no enter standby;1:enter standby*/

    for(tuner_id = 0;tuner_id < 5;tuner_id++)
    {
        memset(&fe_attr, 0x00, sizeof(mt_unf_fe_attr_t));
        mt_unf_fe_get_attr(tuner_id,&fe_attr);
        switch(fe_attr.demod_dev_type)
        {
            case MT_UNF_DEMOD_DEV_TYPE_M88CS8800:
                /* The cs8800 may have two tuner id for DVBS and DVBC,
                *  but it only needs to enter the standby once
                */
                if(cs8800_standby_enable_flag == 0)
                {
                    mt_unf_fe_standby(tuner_id);
                    cs8800_standby_enable_flag = 1;
                    printf("Tuner_id:%d demod_type:%d enter standby\n",tuner_id,fe_attr.demod_dev_type);
                }
                break;
            case MT_UNF_DEMOD_DEV_TYPE_M88RS6060:
            case MT_UNF_DEMOD_DEV_TYPE_SONY_CXD2856:
                mt_unf_fe_standby(tuner_id);
                printf("Tuner_id:%d demod_type:%d enter standby\n",tuner_id,fe_attr.demod_dev_type);
                break;
            default:
                break;
        }
    }
}

/*****************************************************************************
*brief Enter Real  standby
*param  void
*return::MT_SUCCESS             Success.
*return::MT_FAILURE             Fail.
*****************************************************************************/
static void MT_Power_irStartStandby(MT_VOID)
{
    mt_s32 s32Ret = 0;
    sty_wakeup_conf_t wconfig = { 0 };
	MT_UNF_SND_ATTR_S stAttr = { 0 };

	SAMPLE_POWER_IR_INFO_PRINT("close sound!\n");
    s32Ret = MT_UNF_SND_Open(MT_UNF_SND_0, &stAttr);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_POWER_IR_ERR_PRINT(" MT_UNF_SND_Open failed.\n");
        MT_UNF_SND_Close(MT_UNF_SND_0);
        return s32Ret;
    }

    s32Ret = MT_UNF_SND_SetAdacOnOff(MT_UNF_SND_0, 0);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_POWER_IR_ERR_PRINT(" MT_UNF_SND_DeInit failed.\n");
        return s32Ret;
    }

    s32Ret = MT_UNF_SND_Close(MT_UNF_SND_0);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_POWER_IR_ERR_PRINT(" MT_UNF_SND_Close failed.\n");
        return s32Ret;
    }

    SAMPLE_POWER_IR_INFO_PRINT("Low power test\n");
    s32Ret = MT_UNF_PMOC_SetDevType(3);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_POWER_IR_ERR_PRINT("MT_UNF_PMOC_SetDevType failed\n");
    }

    SAMPLE_POWER_IR_INFO_PRINT("MT_UNF_PMOC_SetDevType 3..\n");
    //SAMPLE_POWER_IR_INFO_PRINT("set weak up time \n");

    //wconfig.w_mode = TIME_WAKE_UP;
    /*Support standby time seconds, minutes, hours, days*/
    //wconfig.w_time.wakeup_sec = 5;
    //wconfig.w_time.wakeup_min;
    //wconfig.w_time.wakeup_hour;
    //wconfig.w_time.pass_day = 10000;
    wconfig.w_key.fp_wkey = 0x1;

	SAMPLE_POWER_IR_INFO_PRINT("MT_UNF_PMOC_SetGpenPin 0..\n");
	s32Ret = MT_UNF_PMOC_SetGpenPin(0);	
	if(MT_SUCCESS != s32Ret)
	{
		SAMPLE_POWER_IR_ERR_PRINT("MT_UNF_PMOC_SetGpenPin  failed\n");
	}

    s32Ret = MT_UNF_PMOC_SetWakeUpAttr(wconfig);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_POWER_IR_ERR_PRINT("MT_UNF_PMOC_SetWakeUpAttr  failed\n");
    }
	MT_Power_Frontend_enter_standby_sym6();
	s32Ret = MTADP_Switch_Standby_Mode();
	if(MT_SUCCESS != s32Ret)
	{
		SAMPLE_POWER_IR_ERR_PRINT("MTADP_Switch_Standby_Mode  failed\n");
	}


    SAMPLE_POWER_IR_INFO_PRINT(" start low power......\n");
    s32Ret = MT_UNF_PMOC_SwitchSystemMode();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_POWER_IR_ERR_PRINT("MT_UNF_PMOC_SwitchSystemMode  failed\n");
    }


}

/*****************************************************************************
*brief Monitor the status of keys
*param[in] args, void
*return::void
*****************************************************************************/
static mt_s32 MT_Power_irReceiveTask(void *param)
{
    mt_s32 Ret =  0;
    MT_UNF_KEY_STATUS_E press_status = { 0 };
    MT_U64 u64KeyId = 0;
    char name[64] = { 0 };
    ir_wavefilter_config_s wavefiler = { 0 };
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    MT_U8 protocol = IRDA_NEC;
#elif defined CONFIG_MT_CHIP_SYMPHONY6
    MT_U8 protocol = RC_PROTO_NEC_;
    irda_protocol_t tmpx[]={RC_PROTO_NEC_,RC_PROTO_NECX_,RC_PROTO_RCMM32_,RC_PROTO_RC5_};

#endif
    MT_U32         standbyInfo = 0;
    standby_time_t getTime = { 0 };


    /** Gets the standby wake state, 0-Normal wake-up, 1-ir wake-up, 2-frontpanel wake-up, 3-timer wake-up */
    Ret = MT_UNF_PMOC_GetStandbyInfo(&standbyInfo);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_POWER_IR_ERR_PRINT("MT_UNF_PMOC_GetStandbyInfo failed, ret = %d\n", Ret);
    }

    if(1 == standbyInfo)
    {
        /** Gets standby time */
        Ret = MT_UNF_PMOC_GetStandbyTime(&getTime);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_POWER_IR_ERR_PRINT("MT_UNF_PMOC_GetStandbyTime failed, ret = %d\n", Ret);
            return Ret;
        }
        SAMPLE_POWER_IR_INFO_PRINT("Get standby time: day:%d hour:%d min:%d sec:%d\n", getTime.pass_day, getTime.cur_hour, getTime.cur_min, getTime.cur_sec);
        SAMPLE_POWER_IR_INFO_PRINT("Wake up successful!\n");
    }
    else
    {
        SAMPLE_POWER_IR_INFO_PRINT("Normal boot successful!\n");
    }

    SAMPLE_POWER_IR_INFO_PRINT("Use the power button of the remote control to  Real standby mode\n");
    while(g_bTaskQuit != MT_TRUE)
    {

        Ret = MT_UNF_IR_GetValueWithProtocol(&press_status, &u64KeyId, name, sizeof(name),3000);
        if(MT_SUCCESS == Ret)
        {
            SAMPLE_POWER_IR_INFO_PRINT("u64KeyId = 0x%llx\n", u64KeyId);

            wavefiler.irda_protocol = protocol;
            wavefiler.irda_wfilt_channel = 2;


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
            if ((press_status == 2) && ((u64KeyId == 0xf50a7f80) || (u64KeyId == 0xb748fd01)))
#elif defined CONFIG_MT_CHIP_SYMPHONY6

            MT_UNF_IR_Config_Protocols_ByType(tmpx,4,1);
            wavefiler.irda_wfilt_channel_cfg[1].protocol = RC_PROTO_NEC_;
            wavefiler.irda_wfilt_channel_cfg[1].addr_len = 32;
            wavefiler.irda_wfilt_channel_cfg[1].wfilt_code = 0x800a;

            wavefiler.irda_wfilt_channel_cfg[0].protocol = RC_PROTO_NECX_;
            wavefiler.irda_wfilt_channel_cfg[0].addr_len = 32;
            wavefiler.irda_wfilt_channel_cfg[0].wfilt_code = 0x1fd48;



            (MT_VOID)MT_UNF_IR_SetWaveFilter(&wavefiler);
            if ((press_status == 2) && ((u64KeyId == 0x800a) || (u64KeyId == 0x1fd48)))
#endif
            {
                (MT_VOID)MT_Power_irStartStandby();
            }
        }
    }
    return 0;
}

static MT_VOID MT_Power_irPrintMenu(void)
{

#ifdef MT_SAMPLE_APP
    SAMPLE_POWER_IR_PRINT("     b : background run \n");
#endif
    SAMPLE_POWER_IR_PRINT("     h : help \n");
    SAMPLE_POWER_IR_PRINT("     q : quit \n");
    SAMPLE_POWER_IR_PRINT("POWER_IR>> ");

}

static void MT_Power_irCmdTask(void)
{
    char *fgetret = NULL;
    MT_CHAR inputCmd[32] = { 0 };

    while (1)
    {
        (MT_VOID)MT_Power_irPrintMenu();
        fgetret = fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        fgetret = fgetret;


        if ('q' == inputCmd[0])
        {
            SAMPLE_POWER_IR_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if('b' == inputCmd[0])
        {
            SAMPLE_POWER_IR_INFO_PRINT("ir in back!\n");
            break;
        }
#endif
        else if('h' == inputCmd[0])
        {
            SAMPLE_POWER_IR_INFO_PRINT("help info\n");
        }
    }
}
static MT_VOID MT_Power_irExit(MT_VOID)
{
    g_bTaskQuit = MT_TRUE;

    (MT_VOID)pthread_join(g_stPower_irRunInfo.key, NULL);

    (MT_VOID)MT_UNF_PMOC_DeInit();

    (MT_VOID)MT_UNF_IR_DeInit();
    memset(&g_stPower_irRunInfo, 0xff, sizeof(g_stPower_irRunInfo));
}

static mt_s32 MT_Power_irParase_args(int argc, char *argv[])
{
    int opt = 0;


    SAMPLE_POWER_IR_FUNCTION_ENTER();

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
                    (MT_VOID)MT_Power_irExit();
                }
                return MT_TASK_EXIT;


            default:
                return MT_FAILURE;
            break;
        }
    }
    SAMPLE_POWER_IR_FUNCTION_EXIT();
    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_Power_IrMain(MT_S32 argc, MT_CHAR *argv[])
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
    ret = MT_Power_irParase_args(argc, argv);
    if (MT_FAILURE == ret)
    {
        SAMPLE_POWER_IR_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_POWER_IR_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {

#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_POWER_IR_ERR_PRINT("MT_SYS_Init failed.\n");
            return ret;
        }
#endif

        ret = MT_UNF_PMOC_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_POWER_IR_ERR_PRINT("MT_UNF_PMOC_Init failed\n");
            goto ERR0;
        }

        ret = MT_UNF_IR_Init();
        if (MT_SUCCESS != ret)
        {
            SAMPLE_POWER_IR_ERR_PRINT("MT_UNF_IR_Init ret = %d\n", ret);
            goto ERR1;
        }

        ret = MT_UNF_IR_SetRepKeyTimeoutAttr(300);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_POWER_IR_ERR_PRINT("MT_UNF_IR_SetRepKeyTimeoutAttr ret = %d\n", ret);
            goto ERR2;
        }

        ret = MT_UNF_IR_EnableKeyUp(MT_TRUE);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_POWER_IR_ERR_PRINT("MT_UNF_IR_EnableKeyUp ret = %d\n", ret);
            goto ERR2;
        }

        ret = MT_UNF_IR_EnableRepKey(MT_FALSE);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_POWER_IR_ERR_PRINT("MT_UNF_IR_EnableRepKey ret = %d\n", ret);
            goto ERR2;
        }

        ret = MT_UNF_IR_SetFetchMode(0);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_POWER_IR_ERR_PRINT("MT_UNF_IR_SetFetchMode ret = %d\n", ret);
            goto ERR2;
        }
#ifdef CONFIG_MT_CHIP_SYMPHONY4
        ret = MT_UNF_IR_Enable(MT_TRUE, IRDA_NEC);
#elif defined CONFIG_MT_CHIP_SYMPHONY6
        ret = MT_UNF_IR_Enable(MT_TRUE, RC_PROTO_NEC_);
#endif
        if (MT_SUCCESS != ret)
        {
            SAMPLE_POWER_IR_ERR_PRINT("MT_UNF_IR_SetFetchMode ret = %d\n", ret);
            goto ERR2;
        }
        while(MT_SUCCESS == MT_UNF_IR_GetValueWithProtocol(&press_status, &u64KeyId,name, sizeof(name),1000)) //Clear IR DataCache
        {
            MT_USLEEP(5000);
        }
        g_bTaskQuit = MT_FALSE;
        ret = pthread_create(&g_stPower_irRunInfo.key, NULL, (void * (*)(void *))MT_Power_irReceiveTask, NULL);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_POWER_IR_ERR_PRINT("ErrorCode=0x%x\n",ret);
            goto ERR2;
        }
    }


    (MT_VOID)MT_Power_irCmdTask();
    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

    g_bTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_stPower_irRunInfo.key, NULL);

ERR2:
    (MT_VOID)MT_UNF_IR_DeInit();

ERR1:
    (MT_VOID)MT_UNF_PMOC_DeInit();

ERR0:
#ifndef MT_SAMPLE_APP
    (MT_VOID)mt_sys_deinit();
#endif
    memset(&g_stPower_irRunInfo, 0xff, sizeof(g_stPower_irRunInfo));
    return MT_SUCCESS;
}

