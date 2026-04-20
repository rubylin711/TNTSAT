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
#include "mt_adp_hdmi.h"
#include "mt_adp_frontend.h"
#include "mt_adp_config.h"
#include "mt_cmdline.h"
#ifdef CFG_MT_SAMPLE_NAGRA
#include "mt_unf_flash.h"
#include "mt_unf_misc.h"
#include "mt_unf_hdcp.h"
#endif
/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_STR_DEBUG

#define MT_STR_PRINT   printf
#else

#define MT_STR_PRINT

#endif

#define SAMPLE_STR_FUNCTION_ENTER()            MT_STR_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_STR_FUNCTION_EXIT()             MT_STR_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_STR_FATAL_PRINT(fmt...)          MT_STR_PRINT(" [FATAL] " fmt)
#define SAMPLE_STR_ERR_PRINT(fmt...)            MT_STR_PRINT(" [ERROR] " fmt)
#define SAMPLE_STR_WARN_PRINT(fmt...)           MT_STR_PRINT(" [WARN] "  fmt)
#define SAMPLE_STR_INFO_PRINT(fmt...)           MT_STR_PRINT(" [INFO] "  fmt)
#define SAMPLE_STR_DBG_PRINT(fmt...)            MT_STR_PRINT(" [DEBUG] " fmt)

#define SAMPLE_STR_PRINT printf

#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2

#ifdef CONFIG_MT_CHIP_SYMPHONY4
#define KEY_STANDBY_STR_OLD 0xf50a7f80

#define KEY_STANDBY_STR_NEW 0xb748fd01
#elif defined CONFIG_MT_CHIP_SYMPHONY6
#define KEY_STANDBY_STR_OLD 0x800a

#define KEY_STANDBY_STR_NEW 0x1fd48
#endif
/*************************** Structure Definition ****************************/


typedef struct
{
    pthread_t  key;
} MT_Str_RUN_INFO;



/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;
static MT_Str_RUN_INFO    g_StrRunInfo;
extern cec_config_t g_cec_cfg;
extern MT_BOOL g_str_enable;
/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_Str_standbyMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif
#ifdef CFG_MT_SAMPLE_NAGRA
static mt_s32 nagra_read_hdcp_key(mt_u8 *p_key, mt_u32 key_len)
{
    MT_U32 mtddev = 8;//DEFAULT_MTDDEV;
    char mtddev_name[64] = {0};
    MT_HANDLE gmtd_handle = 0;
    mt_s32 ret = MT_FAILURE;

	mt_unf_flash_init();
	sprintf(mtddev_name,"/dev/mtd%d",mtddev);
	ret = mt_unf_flash_open(mtddev_name, &gmtd_handle);
	if (MT_SUCCESS == ret)
	{
	    ret = mt_unf_flash_read(gmtd_handle, 0, p_key, key_len);
	}
	mt_unf_flash_close(gmtd_handle);
    return ret;
}
#endif
/*****************************************************************************
*brief Enter Real  standby
*param  void
*return::MT_SUCCESS             Success.
*return::MT_FAILURE             Fail.
*****************************************************************************/
static mt_s32 MT_Str_OpenModule(void)
{
    mt_s32 s32Ret = MT_SUCCESS;
    config_t config = { 0 };

    s32Ret = mt_sys_init();
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_ERR_PRINT("mt_sys_init error. ret=0x%x \n", s32Ret);
        return s32Ret;
    }

    s32Ret = MTADP_Read_All_Config(&config);
    if((MT_SUCCESS != s32Ret) || (MT_TRUE != config.is_use))
    {
        SAMPLE_STR_ERR_PRINT("MTADP_Flash_Read failed, ret = %x\n", s32Ret);
        MTADP_Config_Set_Default(&config);
    }

    (mt_void)MTADP_HDMI_Set_HdcpEnable(config.hdcp);

    s32Ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, config.disp_fmt.format);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_ERR_PRINT("MTADP_HDMI_Init failed, ret = %x\n", s32Ret);
        goto ERR0;
    }

    /** Display initialization */
    s32Ret = MTADP_Disp_Init(config.disp_fmt.format);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_ERR_PRINT("MTADP_Disp_Init failed, ret = %x\n", s32Ret);
        goto ERR1;
    }

    return s32Ret;

ERR1:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
ERR0:
    (MT_VOID)mt_sys_deinit();

    return s32Ret;
}

static mt_s32 MT_Str_CloseModule(void)
{
    (MT_VOID)MTADP_Disp_DeInit();

    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);

    (MT_VOID)mt_sys_deinit();

    return MT_SUCCESS;
}

static void MT_Str_StartStandby(MT_VOID)
{
    mt_s32 s32Ret = MT_SUCCESS;
    sty_wakeup_conf_t wconfig = { 0 };

    SAMPLE_STR_INFO_PRINT("Low power test for str\n");

    s32Ret = MT_UNF_PMOC_Init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_ERR_PRINT("MT_UNF_PMOC_Init failed\n");
        return;
    }

    wconfig.w_key.fp_wkey = 0x1;
    s32Ret = MT_UNF_PMOC_SetWakeUpAttr(wconfig);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_ERR_PRINT("MT_UNF_PMOC_SetWakeUpAttr  failed\n");
    }

    /*First close hdmi/disp/sys module, otherwise can not enter str suspend.*/
    (MT_VOID)MT_Str_CloseModule();

    /*Check whether the udhcpc process exists, and kill it if it does.*/
    s32Ret = system("pidof udhcpc");
    if(MT_SUCCESS == s32Ret)
    {
        s32Ret = system("kill $(pidof udhcpc)");
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_STR_ERR_PRINT("kill udhcpc failed\n");
        }
    }

    s32Ret = system("killall -SIGUSR1 audio_ta_service");
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_ERR_PRINT("killall -SIGUSR1 audio_ta_service failed\n");
    }

    s32Ret = system("echo userspace > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor");
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_ERR_PRINT("echo userspace > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor failed\n");
    }

    mt_msleep(200);
#ifdef CONFIG_MT_CHIP_SYMPHONY6
    s32Ret = system("echo 960000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed");
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_ERR_PRINT("echo 960000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed failed\n");
    }
#else
    //symphony4
    s32Ret = system("echo 720000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed");
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_ERR_PRINT("echo 720000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed failed\n");
    }
#endif

    SAMPLE_STR_INFO_PRINT("enter system suspend \n");
    s32Ret = system("echo mem > /sys/power/state");
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_ERR_PRINT("echo mem > /sys/power/state failed\n");
    }

    SAMPLE_STR_INFO_PRINT("exit system suspend \n");
#ifdef CFG_MT_SAMPLE_NAGRA
	{
		u8 key[400];
		int ret = nagra_read_hdcp_key(key,304);
		if(0 == ret){
			printf("-----%x,%x,%x,%x\n",key[0],key[1],key[2],key[3]);
			MT_UNF_HDCP_load_hdcpkey(key);
		} else {
			printf("read hdcp key error\n");
		}
	}
#endif
    s32Ret = system("optee_load_avfw -l 1 -f /usr/local/stb/avfw/avfw.bin");
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_ERR_PRINT("optee_load_avfw -l 1 -f /usr/local/stb/avfw/avfw.bin failed\n");
    }

    s32Ret = system("killall -SIGUSR2 audio_ta_service");
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_ERR_PRINT("killall -SIGUSR2 audio_ta_service failed\n");
    }

#if 0
    s32Ret = system("echo performance > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor");
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_ERR_PRINT("echo performance > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor failed\n");
    }
#else
	//fix bug31220
	s32Ret = system("echo 960000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq");
	s32Ret |= system("echo 1200000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq");
	s32Ret |= system("echo performance > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor");
	if(MT_SUCCESS != s32Ret)
	{
		SAMPLE_STR_ERR_PRINT("echo performance > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor failed\n");
	}
	//printf("720->1200.\n");
#endif
    s32Ret = MT_Str_OpenModule();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_STR_ERR_PRINT("call MT_Str_OpenModule failed\n");
    }

    SAMPLE_STR_INFO_PRINT("%s: system resume success.\n", __FUNCTION__);
}

/*****************************************************************************
*brief Monitor the status of keys
*param[in] args, void
*return::void
*****************************************************************************/
static mt_s32 MT_Str_ReceiveTask(void *param)
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


    SAMPLE_STR_INFO_PRINT("Use the power button of the remote control to  Str standby mode and wakeup\n");
    while(g_bTaskQuit != MT_TRUE)
    {
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
        MT_UNF_IR_Config_Protocols_ByType(tmpx,4,1);
        wavefiler.irda_wfilt_channel_cfg[1].protocol = RC_PROTO_NECX_;
        wavefiler.irda_wfilt_channel_cfg[1].addr_len = 32;
        wavefiler.irda_wfilt_channel_cfg[1].wfilt_code = 0x1fd48;

        wavefiler.irda_wfilt_channel_cfg[2].protocol = RC_PROTO_RCMM32_;
        wavefiler.irda_wfilt_channel_cfg[2].addr_len = 32;
        wavefiler.irda_wfilt_channel_cfg[2].wfilt_code = 0x29c0260c;

        wavefiler.irda_wfilt_channel_cfg[0].protocol = RC_PROTO_NEC_;
        wavefiler.irda_wfilt_channel_cfg[0].addr_len = 32;
        wavefiler.irda_wfilt_channel_cfg[0].wfilt_code = 0x800a;

        (MT_VOID)MT_UNF_IR_SetWaveFilter(&wavefiler);
#endif

        Ret = MT_UNF_IR_GetValueWithProtocol(&press_status, &u64KeyId, name, sizeof(name),3000);
        if(MT_SUCCESS == Ret)
        {
            SAMPLE_STR_INFO_PRINT("u64KeyId = 0x%llx\n", u64KeyId);
            if ((press_status == 2) && ((u64KeyId == KEY_STANDBY_STR_OLD) || (u64KeyId == KEY_STANDBY_STR_NEW)))
            {
                (MT_VOID)MT_Str_StartStandby();
            }
        }
    }
    return 0;
}

static MT_VOID MT_Str_PrintMenu(void)
{
#ifdef MT_SAMPLE_APP
    SAMPLE_STR_PRINT("     b : background run \n");
#endif
    SAMPLE_STR_PRINT("     h : help \n");
    SAMPLE_STR_PRINT("     q : quit \n");
    SAMPLE_STR_PRINT("str_standby>> ");

}

static void MT_Str_CmdTask(void)
{
    MT_CHAR inputCmd[32] = { 0 };

    while (1)
    {
        (MT_VOID)MT_Str_PrintMenu();
        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        if ('q' == inputCmd[0])
        {
            SAMPLE_STR_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if('b' == inputCmd[0])
        {
            SAMPLE_STR_INFO_PRINT("str in back!\n");
            break;
        }
#endif
        else if('h' == inputCmd[0])
        {
            SAMPLE_STR_INFO_PRINT("help info\n");
        }
    }
}
static MT_VOID MT_Str_Exit(MT_VOID)
{
    g_bTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_StrRunInfo.key, NULL);
    if(g_cec_cfg.cec_enable == 0)
    {
        (MT_VOID)MT_UNF_IR_DeInit();
    }
    g_str_enable = MT_FALSE;
    memset(&g_StrRunInfo, 0xff, sizeof(g_StrRunInfo));
}

static mt_s32 MT_StrParase_args(int argc, char *argv[])
{
    int opt = 0;


    SAMPLE_STR_FUNCTION_ENTER();

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
                    (MT_VOID)MT_Str_Exit();
                }
                return MT_TASK_EXIT;


            default:
                return MT_FAILURE;
            break;
        }
    }
    SAMPLE_STR_FUNCTION_EXIT();
    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_Str_standbyMain(MT_S32 argc, MT_CHAR *argv[])
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
    ret = MT_StrParase_args(argc, argv);
    if (MT_FAILURE == ret)
    {
        SAMPLE_STR_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_STR_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
        g_str_enable = MT_TRUE;
        if(g_cec_cfg.cec_enable == 0)
        {
            ret = MT_UNF_IR_Init();
            if (MT_SUCCESS != ret)
            {
                SAMPLE_STR_ERR_PRINT("MT_UNF_IR_Init ret = %d\n", ret);
                goto END;
            }
        }
        ret = MT_UNF_IR_SetRepKeyTimeoutAttr(300);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_STR_ERR_PRINT("MT_UNF_IR_SetRepKeyTimeoutAttr ret = %d\n", ret);
            goto END;
        }

        ret = MT_UNF_IR_EnableKeyUp(MT_TRUE);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_STR_ERR_PRINT("MT_UNF_IR_EnableKeyUp ret = %d\n", ret);
            goto END;
        }

        ret = MT_UNF_IR_EnableRepKey(MT_FALSE);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_STR_ERR_PRINT("MT_UNF_IR_EnableRepKey ret = %d\n", ret);
            goto END;
        }

        ret = MT_UNF_IR_SetFetchMode(0);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_STR_ERR_PRINT("MT_UNF_IR_SetFetchMode ret = %d\n", ret);
            goto END;
        }
#ifdef CONFIG_MT_CHIP_SYMPHONY4
        ret = MT_UNF_IR_Enable(MT_TRUE, IRDA_NEC);
#elif defined CONFIG_MT_CHIP_SYMPHONY6
        ret = MT_UNF_IR_Enable(MT_TRUE, RC_PROTO_NEC_);
#endif
        if (MT_SUCCESS != ret)
        {
            SAMPLE_STR_ERR_PRINT("MT_UNF_IR_SetFetchMode ret = %d\n", ret);
            goto END;
        }
        while(MT_SUCCESS == MT_UNF_IR_GetValueWithProtocol(&press_status, &u64KeyId,name, sizeof(name),1000)) //Clear IR DataCache
        {
            MT_USLEEP(5000);
        }
        g_bTaskQuit = MT_FALSE;
        ret = pthread_create(&g_StrRunInfo.key, NULL, (void * (*)(void *))MT_Str_ReceiveTask, NULL);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_STR_ERR_PRINT("ErrorCode=0x%x\n",ret);
            goto END;
        }
    }


    (MT_VOID)MT_Str_CmdTask();

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

    g_bTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_StrRunInfo.key, NULL);

END:
    if(g_cec_cfg.cec_enable == 0)
    {
        (MT_VOID)MT_UNF_IR_DeInit();
    }

    g_str_enable = MT_FALSE;
    memset(&g_StrRunInfo, 0xff, sizeof(g_StrRunInfo));
    return MT_SUCCESS;
}

